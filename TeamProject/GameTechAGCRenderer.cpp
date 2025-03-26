#include "GameTechAGCRenderer.h"
#include "GameObject.h"
#include "RenderObject.h"
#include "Camera.h"
#include "TextureLoader.h"
#include "MshLoader.h"
#include "../PS5Core/AGCMesh.h"
#include "../PS5Core/AGCTexture.h"
#include "../PS5Core/AGCShader.h"

#include "../CSC8503CoreClasses/Debug.h"

#include "Material.h"
#include "Colors.h"

using namespace NCL;
using namespace Rendering;
using namespace CSC8503;
using namespace PS5;

const int FRAMES_IN_FLIGHT	= 2;

const int BINDLESS_TEX_COUNT		= 128;
const int BINDLESS_BUFFER_COUNT		= 128;

GameTechAGCRenderer::FrameBuffer GameTechAGCRenderer::createBuffer(const std::string& name, FrameBuffer::Slot slot)
{
	FrameBuffer buf;
	buf.target = CreateColourBufferTarget(window->GetScreenSize().x, window->GetScreenSize().y, true);
	buf.target.setSlot((int)slot);
	buf.texture = CreateFrameBufferTextureSlot(name);

	checkError(sce::Agc::Core::translate(buf.texture->GetAGCPointer(), &buf.target, sce::Agc::Core::RenderTargetComponent::kData));

	buf.sampler.init().setXyFilterMode(
		sce::Agc::Core::Sampler::FilterMode::kPoint,
		sce::Agc::Core::Sampler::FilterMode::kPoint
	).setMipFilterMode(sce::Agc::Core::Sampler::MipFilterMode::kPoint);

	return buf;
}

void GameTechAGCRenderer::useViewPort(sce::Agc::Core::BasicContext* context, Vector2i size) const
{
	sce::Agc::CxViewport viewPort;
	sce::Agc::Core::setViewport(&viewPort, size.x, size.y, 0, 0, -1.0f, 1.0f);
	context->m_sb.setState(viewPort);
}

void GameTechAGCRenderer::prepPostProcessing(sce::Agc::CxRenderTarget& target)
{
	sce::Agc::CxRenderTargetMask rtMask = sce::Agc::CxRenderTargetMask().init().setMask(0, 0xFF);
	frameContext->m_sb.setState(rtMask);
	frameContext->m_sb.setState(target);

	sce::Agc::CxBlendControl blendControl;
	blendControl.init().setBlend(sce::Agc::CxBlendControl::Blend::kDisable);
	frameContext->m_sb.setState(blendControl);

	frameContext->m_sb.setState(sce::Agc::CxPrimitiveSetup().init().setCullFace(sce::Agc::CxPrimitiveSetup::CullFace::kBack));

	sce::Agc::CxDepthStencilControl depthControl;
	depthControl.init().setDepth(sce::Agc::CxDepthStencilControl::Depth::kDisable).setDepthFunction(sce::Agc::CxDepthStencilControl::DepthFunction::kAlways);
	frameContext->m_sb.setState(depthControl);
}

GameTechAGCRenderer::GameTechAGCRenderer(Window* window) : AGCRenderer(window), GameTechRendererInterface(window) {
	bindlessTextures = (sce::Agc::Core::Texture*)allocator.Allocate(BINDLESS_TEX_COUNT * sizeof(sce::Agc::Core::Texture), sce::Agc::Alignment::kBuffer);
	sce::Agc::Core::BufferSpec texSpec;
	texSpec.initAsRegularBuffer(bindlessTextures, sizeof(sce::Agc::Core::Texture), BINDLESS_TEX_COUNT);
	checkError(sce::Agc::Core::initialize(&textureBuffer, &texSpec));

	bindlessBuffers = (sce::Agc::Core::Buffer*)allocator.Allocate(BINDLESS_BUFFER_COUNT * sizeof(sce::Agc::Core::Buffer), sce::Agc::Alignment::kBuffer);
	sce::Agc::Core::BufferSpec buffSpec;
	buffSpec.initAsRegularBuffer(bindlessBuffers, sizeof(sce::Agc::Core::Buffer), BINDLESS_BUFFER_COUNT);
	checkError(sce::Agc::Core::initialize(&arrayBuffer, &buffSpec));
	bufferCount = 1; //We skip over index 0, makes some selection logic easier later

	unitQuad = Mesh::Quad<AGCMesh>(1.0f);
	unitQuad->UploadToGPU(this);
	halfUnitQuad = Mesh::Quad<AGCMesh>(0.5f);
	halfUnitQuad->UploadToGPU(this);

	sphere = std::unique_ptr<AGCMesh>((AGCMesh*)LoadMesh("Sphere.msh"));
	highResSphere = std::unique_ptr<PS5::AGCMesh>((AGCMesh*)LoadMesh("Sphere_HighRes.msh"));

	gammaCompute	= std::make_unique<AGCShader>("Gamma_c.ags", allocator);

	defaultVertexShader = std::make_unique<AGCShader>("Tech_vv.ags", allocator);
	defaultPixelShader  = std::make_unique<AGCShader>("Tech_p.ags"  , allocator);

	uiVertexShader = std::make_unique<AGCShader>("UI_vv.ags", allocator);
	uiPixelShader = std::make_unique<AGCShader>("UI_p.ags", allocator);

	debugLineVertexShader	= std::make_unique<AGCShader>("DebugLine_vv.ags", allocator);
	debugLinePixelShader	= std::make_unique<AGCShader>("DebugLine_p.ags" , allocator);

	debugTextVertexShader	= std::make_unique<AGCShader>("DebugText_vv.ags", allocator);
	debugTextPixelShader	= std::make_unique<AGCShader>("DebugText_p.ags" , allocator);

	deferredVertexShader = std::make_unique<AGCShader>("deferred_vv.ags", allocator);
	deferredPixelShader = std::make_unique<AGCShader>("deferred_p.ags", allocator);

	postVertexShader = std::make_unique<AGCShader>("post_vv.ags", allocator);
	postPixelShader = std::make_unique<AGCShader>("post_p.ags", allocator);

	laserVertexShader = std::make_unique<AGCShader>("laser_vv.ags", allocator);
	laserPixelShader = std::make_unique<AGCShader>("laser_p.ags", allocator);
	laserPreShader = std::make_unique<AGCShader>("laser_pre_p.ags", allocator);

	decalVertexShader = std::make_unique<AGCShader>("decal_vv.ags", allocator);
	decalPixelShader = std::make_unique<AGCShader>("decal_p.ags", allocator);

	allFrames = new FrameData[FRAMES_IN_FLIGHT];
	for (int i = 0; i < FRAMES_IN_FLIGHT; ++i) {

		{//We store scene object matrices etc in a big UBO
			allFrames[i].data.dataStart = (char*)allocator.Allocate(UboSize, sce::Agc::Alignment::kBuffer);
			allFrames[i].data.data = allFrames[i].data.dataStart;

			sce::Agc::Core::BufferSpec bufSpec;
			bufSpec.initAsConstantBuffer(allFrames[i].data.dataStart, sizeof(ShaderConstants));

			checkError(sce::Agc::Core::initialize(&allFrames[i].constantBuffer, &bufSpec));
		}
	}
	currentFrameIndex = 0;
	currentFrame = &allFrames[currentFrameIndex];

	auto tex = std::make_shared<AGCTexture>();
	Debug::CreateDebugFont("Comicy.ttf", tex);

	sceneBuffer = createBuffer("Scene", FrameBuffer::Slot::Color);
	sceneNormalBuffer = createBuffer("SceneNormal", FrameBuffer::Slot::Normal);
	lightDiffuse = createBuffer("LightsDiffuse", FrameBuffer::Slot::Color);
	lightSpecular = createBuffer("LightsSpecular", FrameBuffer::Slot::Specular);
	screenBuffer = createBuffer("Screen", FrameBuffer::Slot::Color);

	laserBuffer = createBuffer("Laser1", FrameBuffer::Slot::Color);
	previousLaserBuffer = createBuffer("Laser2", FrameBuffer::Slot::Color);
}

GameTechAGCRenderer::~GameTechAGCRenderer()	{
}

Mesh* GameTechAGCRenderer::LoadMesh(const std::string& name) {
	AGCMesh* m = new AGCMesh();
	MshLoader::LoadMesh(name, *m);
	m->UploadToGPU(this);
	return m;
}

NCL::PS5::AGCTexture* GameTechAGCRenderer::CreateFrameBufferTextureSlot(const std::string& name) {
	uint32_t index = textureMap.size();
	auto t = new AGCTexture(allocator);
	RegisterTexture(name, t);
	return t;
}

Texture* GameTechAGCRenderer::LoadTexture(const std::string& name) {
	auto found = textureMap.find(name);
	if (found != textureMap.end()) {
		return (Texture*)found->second;
	}
	auto t = new AGCTexture(name, allocator);
	RegisterTexture(name, t);
	return t;
}

void GameTechAGCRenderer::RegisterTexture(const std::string& name, AGCTexture* outTex)
{
	std::unique_lock lock(texMapMtx);
	// FIXME: Handle the texture already being loaded
	outTex->SetAssetID(textureMap.size());
	bindlessTextures[outTex->GetAssetID()] = *outTex->GetAGCPointer();

	textureMap.insert({ name, outTex });
}

void GameTechAGCRenderer::RenderFrame() {
	currentFrame = &allFrames[currentFrameIndex];

	currentFrame->data.Reset();

	//Step 1: Write the frame's constant data to the buffer
	WriteRenderPassConstants();
	//Step 2: Walk the object list and build up the object set and required buffer memory
	UpdateObjectList();
	//Step 4: Go through the geometry and darw it to a shadow map
	//Step 6: Draw the scene to our main scene render target
	MainRenderPass();

	// Step 7: Apply post processing to the scene buffer
	LightPass();
	LaserPass();
	PostProcessPass();

	////Step 8: Draw UI to the post-processed scene
	UiPass();
	UpdateDebugData();
	RenderDebugLines();
	RenderDebugText();

	//Step 9: Draw the main scene render target to the screen with a compute shader
	DisplayRenderPass(); //Puts our scene on screen, uses a compute

	currentFrameIndex = (currentFrameIndex + 1) % FRAMES_IN_FLIGHT;
}

/*
This method builds a struct that

*/
void GameTechAGCRenderer::WriteRenderPassConstants() {
	ShaderConstants frameData;
	frameData.screenSize = Vector2(ScreenSize.x, ScreenSize.y);
	frameData.cameraPos = camera->GetPosition();

	frameData.viewMatrix = camera->BuildViewMatrix();
	frameData.projMatrix = camera->BuildProjectionMatrix(hostWindow->GetScreenAspect());
	frameData.nearPlane = camera->GetNearPlane();
	frameData.farPlane = camera->GetFarPlane();

	frameData.viewProjMatrix = frameData.projMatrix * frameData.viewMatrix;

	frameData.inverseViewProjMatrix = Matrix::Inverse(frameData.viewProjMatrix);
	frameData.inverseViewMatrix = Matrix::Inverse(frameData.viewMatrix);
	frameData.inverseProjMatrix = Matrix::Inverse(frameData.projMatrix);

	frameData.orthoMatrix = Matrix::Orthographic(0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, true);

	frameData.time = vignettePulse;
	frameData.dt = delta;

	frameData.vingetteSettings.enabled = GetVignetteOn();
	frameData.vingetteSettings.color = vignetteColour;
	frameData.vingetteSettings.intensity = vignetteIntensity;
	frameData.vingetteSettings.pulse = vignettePulse;

	frameData.ambient = 0.1f;


	currentFrame->data.WriteData<ShaderConstants>(frameData); //Let's start filling up our frame data!

	currentFrame->data.AlignData(128);
}

void GameTechAGCRenderer::DrawObjects() {
	if (frameObjects.empty()) {
		return;
	}
	uint32_t startingIndex = 0;

	AGCMesh* prevMesh = (AGCMesh*)frameObjects[0]->GetMesh();
	int instanceCount = 0;

	// Draw all instances of the current mesh that have not yet been rendered
	auto drawPendingInstances = [&]() {
		prevMesh->BindVertexBuffers(frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs));
		uint32_t* objID = static_cast<uint32_t*>(frameContext->m_dcb.allocateTopDown(sizeof(uint32_t), sce::Agc::Alignment::kBuffer));
		*objID = startingIndex;
		frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs).setUserSrtBuffer(objID, 1);

		DrawBoundMeshInstanced(*frameContext, *prevMesh, instanceCount);

		startingIndex += instanceCount;
		instanceCount = 0;
	};

	for (auto obj : frameObjects) {
		AGCMesh* objectMesh = (AGCMesh*)obj->GetMesh();

		// If the new mesh is different than previous meshes, flush out the old list
		if (prevMesh != objectMesh) {
			drawPendingInstances();
		}
		prevMesh = objectMesh;
		instanceCount += objectMesh->GetSubMeshCount();
	}

	drawPendingInstances();
	// Check that we used all the buffers we expected
	assert(instanceCount + startingIndex == currentFrame->objects.count);
}

void GameTechAGCRenderer::DrawDecals() {
	if (currentFrame->decals.count <= 0) {
		return;
	}

	frameContext->setShaders(nullptr, decalVertexShader->GetAGCPointer(), decalPixelShader->GetAGCPointer(), sce::Agc::UcPrimitiveType::Type::kTriList);
	// Don't write to the depth buffer, we'll be discarding fragments manually
	frameContext->m_sb.setState(sce::Agc::CxDepthStencilControl().init()
		.setDepth(sce::Agc::CxDepthStencilControl::Depth::kDisable).setDepthWrite(sce::Agc::CxDepthStencilControl::DepthWrite::kDisable)
	);
	frameContext->m_sb.setState(sce::Agc::CxBlendControl().init()
		.setBlend(sce::Agc::CxBlendControl::Blend::kEnable)
		.setColorSourceMultiplier(sce::Agc::CxBlendControl::ColorSourceMultiplier::kSrcAlpha)
		.setColorDestMultiplier(sce::Agc::CxBlendControl::ColorDestMultiplier::kOneMinusSrcAlpha)
	);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setBuffers(0, 1, &currentFrame->decals.buffer);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kPs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setBuffers(0, 1, &textureBuffer)
		.setSamplers(0, 1, &defaultSampler)
		.setSamplers(1, 1, &depthSampler).setTextures(1, 1, depthTexture->GetAGCPointer());

	unitQuad->BindVertexBuffers(frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs));
	DrawBoundMeshInstanced(*frameContext, *unitQuad, currentFrame->decals.count);
}

void GameTechAGCRenderer::MainRenderPass() {
	frameContext->setShaders(nullptr, defaultVertexShader->GetAGCPointer(), defaultPixelShader->GetAGCPointer(), sce::Agc::UcPrimitiveType::Type::kTriList);

	useViewPort(frameContext, ScreenSize);
	frameContext->m_sb.setState(backBuffers[currentSwap].targetMask);
	frameContext->m_sb.setState(backBuffers[currentSwap].renderTarget);

	sce::Agc::CxRenderTargetMask rtMask = sce::Agc::CxRenderTargetMask().init().setMask(0, 0xFF).setMask(1, 0xff);
	frameContext->m_sb.setState(rtMask);
	frameContext->m_sb.setState(sceneBuffer.target);
	frameContext->m_sb.setState(sceneNormalBuffer.target);

	auto result = sce::Agc::Toolkit::clearRenderTargetCs(&frameContext->m_dcb, &sceneBuffer.target);
	result |= sce::Agc::Toolkit::clearRenderTargetCs(&frameContext->m_dcb, &sceneNormalBuffer.target);
	frameContext->resetToolkitChangesAndSyncToGl2(result);

	frameContext->m_sb.setState(depthTarget);

	sce::Agc::CxBlendControl blendControl;
	blendControl.init()
		.setBlend(sce::Agc::CxBlendControl::Blend::kEnable)
		.setColorSourceMultiplier(sce::Agc::CxBlendControl::ColorSourceMultiplier::kSrcAlpha)
		.setColorDestMultiplier(sce::Agc::CxBlendControl::ColorDestMultiplier::kOneMinusSrcAlpha)
		.setColorBlendFunc(sce::Agc::CxBlendControl::ColorBlendFunc::kAdd);
	frameContext->m_sb.setState(blendControl);

	sce::Agc::CxDepthStencilControl depthControl;
	depthControl.init();
	depthControl.setDepth(sce::Agc::CxDepthStencilControl::Depth::kEnable);
	depthControl.setDepthFunction(sce::Agc::CxDepthStencilControl::DepthFunction::kLessEqual);
	depthControl.setDepthWrite(sce::Agc::CxDepthStencilControl::DepthWrite::kEnable);
	frameContext->m_sb.setState(depthControl);

	sce::Agc::CxPrimitiveSetup primitiveSetup;
	primitiveSetup.init()
		.setCullFace(sce::Agc::CxPrimitiveSetup::CullFace::kBack);
	frameContext->m_sb.setState(primitiveSetup);


	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setBuffers(0, 1, &currentFrame->objects.buffer)
		.setBuffers(1, 1, &arrayBuffer);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kPs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setBuffers(0, 1, &textureBuffer)
		.setSamplers(0, 1, &defaultSampler);
	DrawObjects();
	DrawDecals();
}

void GameTechAGCRenderer::UiPass() {
	frameContext->setShaders(nullptr, uiVertexShader->GetAGCPointer(), uiPixelShader->GetAGCPointer(), sce::Agc::UcPrimitiveType::Type::kTriList);

	useViewPort(frameContext, ScreenSize);
	frameContext->m_sb.setState(backBuffers[currentSwap].targetMask);
	frameContext->m_sb.setState(backBuffers[currentSwap].renderTarget);

	sce::Agc::CxRenderTargetMask rtMask = sce::Agc::CxRenderTargetMask().init().setMask(0, 0xFF);
	frameContext->m_sb.setState(rtMask);
	frameContext->m_sb.setState(screenBuffer.target);

	frameContext->m_sb.setState(depthTarget);

	sce::Agc::CxDepthStencilControl depthControl;
	depthControl.init();
	depthControl.setDepth(sce::Agc::CxDepthStencilControl::Depth::kDisable);
	frameContext->m_sb.setState(depthControl);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs)
		.setBuffers(0, 1, &currentFrame->ui.buffer);
	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kPs)
		.setBuffers(0, 1, &textureBuffer)
		.setSamplers(0, 1, &defaultSampler);

	halfUnitQuad->BindVertexBuffers(frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs));
	DrawBoundMeshInstanced(*frameContext, *halfUnitQuad, frameSprites.size());
}

void GameTechAGCRenderer::LightPass()
{
	// Clear the FBO using a compute shader, waits for it to complete before continuing operations to avoid race conditions
	auto result = sce::Agc::Toolkit::clearRenderTargetCs(&frameContext->m_dcb, &lightDiffuse.target, sce::Agc::Toolkit::RenderTargetClearOp::kAuto);
	result |= sce::Agc::Toolkit::clearRenderTargetCs(&frameContext->m_dcb, &lightSpecular.target, sce::Agc::Toolkit::RenderTargetClearOp::kAuto);
	frameContext->resetToolkitChangesAndSyncToGl2(result);

	frameContext->setShaders(nullptr, deferredVertexShader->GetAGCPointer(), deferredPixelShader->GetAGCPointer(), sce::Agc::UcPrimitiveType::Type::kTriList);
	useViewPort(frameContext, ScreenSize);

	sce::Agc::CxRenderTargetMask rtMask;
	rtMask.init().setMask(0, 0xff).setMask(1, 0xff);
	frameContext->m_sb.setState(rtMask)
		.setState(lightDiffuse.target)
		.setState(lightSpecular.target);

	sce::Agc::CxPrimitiveSetup primSetup;
	primSetup.init().setCullFace(sce::Agc::CxPrimitiveSetup::CullFace::kFront);
	frameContext->m_sb.setState(primSetup);

	sce::Agc::CxDepthStencilControl depthControl;
	depthControl.init().setDepth(sce::Agc::CxDepthStencilControl::Depth::kDisable).setDepthFunction(sce::Agc::CxDepthStencilControl::DepthFunction::kAlways);
	frameContext->m_sb.setState(depthControl);

	sce::Agc::CxBlendControl blendControl;
	blendControl.init()
		.setBlend(sce::Agc::CxBlendControl::Blend::kEnable)
		.setColorSourceMultiplier(sce::Agc::CxBlendControl::ColorSourceMultiplier::kOne)
		.setColorDestMultiplier(sce::Agc::CxBlendControl::ColorDestMultiplier::kOne)
		.setColorBlendFunc(sce::Agc::CxBlendControl::ColorBlendFunc::kAdd);
	frameContext->m_sb.setState(blendControl);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setBuffers(0, 1, &currentFrame->lights.buffer);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kPs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setBuffers(0, 1, &currentFrame->lights.buffer)
		.setSamplers(1, 1, &sceneNormalBuffer.sampler).setTextures(1, 1, sceneNormalBuffer.texture->GetAGCPointer())
		.setSamplers(2, 1, &depthSampler).setTextures(2, 1, depthTexture->GetAGCPointer());

	sphere->BindVertexBuffers(frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs));
	DrawBoundMeshInstanced(*frameContext, *sphere, lights.size());
}

void GameTechAGCRenderer::LaserPass()
{
	// Use a faded version of the previos frame's lasers as a base
	frameContext->setShaders(nullptr, postVertexShader->GetAGCPointer(), laserPreShader->GetAGCPointer(), sce::Agc::UcPrimitiveType::Type::kTriList);
	useViewPort(frameContext, ScreenSize);
	prepPostProcessing(laserBuffer.target);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kPs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setSamplers(0, 1, &previousLaserBuffer.sampler).setTextures(0, 1, previousLaserBuffer.texture->GetAGCPointer());
	unitQuad->BindVertexBuffers(frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs));
	DrawBoundMesh(*frameContext, *unitQuad);

	frameContext->setShaders(nullptr, laserVertexShader->GetAGCPointer(), laserPixelShader->GetAGCPointer(), sce::Agc::UcPrimitiveType::Type::kTriList);
	useViewPort(frameContext, ScreenSize);


	sce::Agc::CxRenderTargetMask rtMask = sce::Agc::CxRenderTargetMask().init().setMask(0, 0xFF);
	frameContext->m_sb.setState(rtMask);
	frameContext->m_sb.setState(laserBuffer.target);
	frameContext->m_sb.setState(depthTarget);

	sce::Agc::CxBlendControl blendControl;
	blendControl.init()
		.setBlend(sce::Agc::CxBlendControl::Blend::kEnable)
		.setColorSourceMultiplier(sce::Agc::CxBlendControl::ColorSourceMultiplier::kSrcAlpha)
		.setColorDestMultiplier(sce::Agc::CxBlendControl::ColorDestMultiplier::kOneMinusSrcAlpha)
		.setColorBlendFunc(sce::Agc::CxBlendControl::ColorBlendFunc::kAdd);
	frameContext->m_sb.setState(blendControl);

	sce::Agc::CxDepthStencilControl depthControl;
	depthControl.init();
	depthControl.setDepth(sce::Agc::CxDepthStencilControl::Depth::kEnable);
	depthControl.setDepthFunction(sce::Agc::CxDepthStencilControl::DepthFunction::kLessEqual);
	depthControl.setDepthWrite(sce::Agc::CxDepthStencilControl::DepthWrite::kDisable);
	frameContext->m_sb.setState(depthControl);

	sce::Agc::CxPrimitiveSetup primitiveSetup;
	primitiveSetup.init()
		.setCullFace(sce::Agc::CxPrimitiveSetup::CullFace::kBack);
	frameContext->m_sb.setState(primitiveSetup);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setBuffers(0, 1, &currentFrame->lasers.buffer)
		.setBuffers(1, 1, &arrayBuffer);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kPs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer);

	highResSphere->BindVertexBuffers(frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs));
	DrawBoundMeshInstanced(*frameContext, *highResSphere, lasers.size());

	std::swap(laserBuffer, previousLaserBuffer);
}

void GameTechAGCRenderer::PostProcessPass()
{
	frameContext->setShaders(nullptr, postVertexShader->GetAGCPointer(), postPixelShader->GetAGCPointer(), sce::Agc::UcPrimitiveType::Type::kTriList);
	useViewPort(frameContext, ScreenSize);

	prepPostProcessing(screenBuffer.target);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kPs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setSamplers(0, 1, &sceneBuffer.sampler).setTextures(0, 1, sceneBuffer.texture->GetAGCPointer())
		.setSamplers(1, 1, &lightDiffuse.sampler).setTextures(1, 1, lightDiffuse.texture->GetAGCPointer())
		.setSamplers(2, 1, &lightSpecular.sampler).setTextures(2, 1, lightSpecular.texture->GetAGCPointer())
		.setSamplers(3, 1, &depthSampler).setTextures(3, 1, depthTexture->GetAGCPointer())
		.setSamplers(4, 1, &laserBuffer.sampler).setTextures(4, 1, laserBuffer.texture->GetAGCPointer());

	unitQuad->BindVertexBuffers(frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs));
	DrawBoundMesh(*frameContext, *unitQuad);
}

void GameTechAGCRenderer::UpdateDebugData() {
	const std::vector<NCL::Debug::DebugStringEntry>& strings = NCL::Debug::GetDebugStrings();
	const std::vector<Debug::DebugLineEntry>& lines = Debug::GetDebugLines();

	currentFrame->debugLines.begin(currentFrame);
	for (const auto& line : lines) {
		LineState begin; LineState end;
		begin.pos = Vector4(line.start, 0); begin.colour = line.colourA;
		end.pos = Vector4(line.end, 0); end.colour = line.colourB;
		currentFrame->data.WriteData(&begin, sizeof(begin));
		currentFrame->data.WriteData(&end, sizeof(end));
	}
	currentFrame->debugLines.end(currentFrame);


	currentFrame->debugText.begin(currentFrame);
	std::vector<SimpleFont::InterleavedTextVertex> verts;
	static_assert(sizeof(SimpleFont::InterleavedTextVertex) == sizeof(TextState));
	for (const auto& s : strings) {
		Debug::GetDebugFont()->BuildInterleavedVerticesForString(s.data, s.position, s.colour, s.scale, verts);
		currentFrame->data.WriteData(verts.data(), verts.size() * sizeof(TextState));
		verts.clear();
	}
	currentFrame->debugText.end(currentFrame);
}

void GameTechAGCRenderer::DisplayRenderPass() {
	sce::Agc::Core::gpuSyncEvent(&frameContext->m_dcb, sce::Agc::Core::SyncWaitMode::kDrainGraphics, sce::Agc::Core::SyncCacheOp::kFlushUncompressedColorBufferForTexture);

	frameContext->setCsShader(gammaCompute->GetAGCPointer());

	sce::Agc::DispatchModifier modifier = gammaCompute->GetAGCPointer()->m_specials->m_dispatchModifier;

	sce::Agc::Core::Texture outputTex; //Alias for our framebuffer tex;
	checkError(sce::Agc::Core::translate(&outputTex, &backBuffers[currentSwap].renderTarget, sce::Agc::Core::RenderTargetComponent::kData));

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kCs)
		.setTextures(0, 1, screenBuffer.texture->GetAGCPointer())
		.setRwTextures(1, 1, &outputTex);
	uint32_t xDims = (outputTex.getWidth() + 7) / 8;
	uint32_t yDims = (outputTex.getHeight() + 7) / 8;

	frameContext->m_dcb.dispatch(xDims, yDims, 1, modifier);
}

void GameTechAGCRenderer::RenderDebugLines() {
	if (currentFrame->debugLines.count == 0) {
		return;
	}
	frameContext->setShaders(nullptr, debugLineVertexShader->GetAGCPointer(), debugLinePixelShader->GetAGCPointer(), sce::Agc::UcPrimitiveType::Type::kLineList);
	sce::Agc::CxDepthStencilControl depthControl;
	depthControl.init()
		.setDepth(sce::Agc::CxDepthStencilControl::Depth::kDisable)
		.setDepthWrite(sce::Agc::CxDepthStencilControl::DepthWrite::kDisable);
	frameContext->m_sb.setState(depthControl);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setBuffers(0, 1, &currentFrame->debugLines.buffer);

	frameContext->drawIndexAuto(currentFrame->debugLines.count);
}

void GameTechAGCRenderer::RenderDebugText() {
	if (currentFrame->debugText.count == 0) {
		return;
	}
	frameContext->setShaders(nullptr, debugTextVertexShader->GetAGCPointer(), debugTextPixelShader->GetAGCPointer(), sce::Agc::UcPrimitiveType::Type::kTriList);
	sce::Agc::CxDepthStencilControl depthControl;
	depthControl.init()
		.setDepth(sce::Agc::CxDepthStencilControl::Depth::kDisable)
		.setDepthWrite(sce::Agc::CxDepthStencilControl::DepthWrite::kDisable);
	frameContext->m_sb.setState(depthControl);

	sce::Agc::CxPrimitiveSetup primSetup;
	primSetup.init().setCullFace(sce::Agc::CxPrimitiveSetup::CullFace::kNone);
	frameContext->m_sb.setState(primSetup);

	sce::Agc::CxBlendControl blendControl;
	blendControl.init();
	blendControl.setBlend(sce::Agc::CxBlendControl::Blend::kEnable)
		.setAlphaBlendFunc(sce::Agc::CxBlendControl::AlphaBlendFunc::kAdd)
		.setColorSourceMultiplier(sce::Agc::CxBlendControl::ColorSourceMultiplier::kSrcAlpha)
		.setColorDestMultiplier(sce::Agc::CxBlendControl::ColorDestMultiplier::kOneMinusSrcAlpha)
		.setColorBlendFunc(sce::Agc::CxBlendControl::ColorBlendFunc::kAdd);

	frameContext->m_sb.setState(blendControl);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setBuffers(0, 1, &currentFrame->debugText.buffer);

	AGCTexture* debugTex = (AGCTexture*)Debug::GetDebugFont()->getTexture().get();

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kPs)
		.setSamplers(0, 1, &pixelSampler)
		.setTextures(1, 1, debugTex->GetAGCPointer());

	frameContext->drawIndexAuto(currentFrame->debugText.count);
}

void GameTechAGCRenderer::UpdateObjectList() {
	currentFrame->objects.begin(currentFrame);
	for (auto g : frameObjects) {
		::ObjectState state;
		Matrix4 transMatrix;
		g->getParent()->GetTransform().getOpenGLMatrix((float*)&transMatrix);
		state.modelMatrix = transMatrix * Matrix::Scale(g->getParent()->getRenderScale());
		// Matrix without translation components, for normal transforms
		Matrix3 transMat3 = Matrix::FromMat4(state.modelMatrix);
		state.normalMatrix = Matrix::FromMat3(Matrix::InverseTranspose(transMat3));

		state.texRepeats = g->GetTexRepeating();
		state.texScale = g->getParent()->getRenderScale() * g->GetTexScaleMultiplier();
		state.animJointsIndex = NULLTEX;

		AGCMesh* m = (AGCMesh*)g->GetMesh();
		if (g->GetAnimation()) { //It's a skeleton mesh, need to update transformed vertices buffer
			// FIXME: Don't hardcode offset for the player
			state.modelMatrix = state.modelMatrix * Matrix::Translation(Vector3(0, -0.9f, 0.1f)); //Translation added to centre the player mesh better

			Buffer* b = g->GetGPUBuffer();
			if (!b) {
				//We've not yet made a buffer to hold the matrices of this skeleton
				size_t matCount = m->GetBindPose().size();
				void* data = allocator.Allocate(matCount * sizeof(Matrix4), sce::Agc::Alignment::kBuffer);

				sce::Agc::Core::BufferSpec bufSpec;
				bufSpec.initAsRegularBuffer(data, sizeof(Matrix4), matCount);

				sce::Agc::Core::Buffer vBuffer;
				checkError(sce::Agc::Core::initialize(&vBuffer, &bufSpec));

				uint32_t bufferID = bufferCount++;
				b = new AGCBuffer(vBuffer, data);
				b->SetAssetID(bufferID);
				g->SetGPUBuffer(b);

				bindlessBuffers[bufferID] = vBuffer;
			}
			auto frameData = g->GetAnimation()->GetJointData(g->GetAnimation()->GetCurrentFrame());
			Matrix4* bufPtr = (Matrix4*)((AGCBuffer*)b)->GetAllocatedMemory();
			for (unsigned int q = 0; q < m->GetJointCount(); ++q) {
				const Matrix4 invBindPose = g->GetMesh()->GetInverseBindPose()[q]; //need to get the inverse bind pose per joint to "undo" the bind pose for each joint
				*bufPtr = frameData[q] * invBindPose;
				bufPtr++;
			}

			state.animJointsIndex = b->GetAssetID();
		}

		assert(g->GetMesh()->GetSubMeshCount() > 0);
		for (int i = 0; i < g->GetMesh()->GetSubMeshCount(); i++) {
			auto subMesh = g->GetMesh()->GetSubMesh(i);
			auto layer = g->getMaterial() ? g->getMaterial()->GetLayer(i) : nullptr;
			state.colour = (layer && layer->useColor) ? g->GetColour() : Vector4(1, 1, 1, 1);

			auto t = layer ? layer->diffuse : nullptr;
			state.texIndex = t ? t->GetAssetID() : NULLTEX;
			if (!t) {
				state.colour = Vector4(0, 0, 0, 0);
				std::cout << i << std::endl;
			}

			auto  normal = layer ? layer->normal : nullptr;
			state.normalIndex = normal ? normal->GetAssetID() : NULLTEX;

			state.startIndex = subMesh->start;
			state.numElements = subMesh->count;
			// This behaviour is inverted from OpenGL for compatibility with HLSL UVs
			state.invertY = !(layer && layer->invertY);
			currentFrame->data.WriteData(state);
		}

	}
	currentFrame->objects.end(currentFrame);

	currentFrame->ui.begin(currentFrame);
	for (auto& ui : frameSprites) {
		UiState state;
		state.colour = ui.color;
		state.position = ui.position;
		state.size = ui.size;
		state.texture = ui.texture != nullptr ? ui.texture->GetAssetID() : NULLTEX;
		currentFrame->data.WriteData(state);
	}
	currentFrame->ui.end(currentFrame);

	for (const auto& uiElement : frameTexts) {
		Debug::Print(uiElement.text, uiElement.position, uiElement.color);
	}

	currentFrame->lights.begin(currentFrame);
	for (auto& light : lights) {
		LightState state;
		state.color = light->colour;
		state.position = light->worldPosition;
		state.radius = light->radius;
		state.intensity = light->intensity;
		currentFrame->data.WriteData(state);
	}
	currentFrame->lights.end(currentFrame);

	currentFrame->lasers.begin(currentFrame);
	for (auto& laser : lasers) {
		LaserState state;
		state.start = laser->GetStartPos();
		state.end = laser->GetEndPos();
		state.thickness = 0.25f;
		state.colour = laser->GetColor();
		currentFrame->data.WriteData(state);
	}
	currentFrame->lasers.end(currentFrame);

	currentFrame->decals.begin(currentFrame);
	for (auto& decal : decalSystem.GetDecals()) {
		DecalState state;
		state.modelMatrix = Matrix::Translation(Vector3(decal.position))
			* Matrix::RotationAroundNormal(decal.normal, decal.rotation)
			* Matrix::Scale(Vector3(decal.radius, decal.radius, decal.radius));
		state.color = decal.color;
		state.fade = decal.alphaFade;
		state.textureId = decal.texture != nullptr ? decal.texture->GetAssetID() : NULLTEX;
		currentFrame->data.WriteData(state);
	}
	currentFrame->decals.end(currentFrame);
}

template<typename T>
inline void GameTechAGCRenderer::FrameData::UniformArray<T>::begin(FrameData* frame)
{
	start = frame->data.data;
}

template<typename T>
inline void GameTechAGCRenderer::FrameData::UniformArray<T>::end(FrameData* frame)
{
	sce::Agc::Core::BufferSpec spec;
	auto bytesWritten = frame->data.data - start;
	count = bytesWritten / sizeof(T);
	spec.initAsRegularBuffer(start, sizeof(T), count);
	sce::Agc::Core::initialize(&buffer, &spec);
}
