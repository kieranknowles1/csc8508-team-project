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

using namespace NCL;
using namespace Rendering;
using namespace CSC8503;
using namespace PS5;

const int FRAMES_IN_FLIGHT	= 2;

const int BINDLESS_TEX_COUNT		= 128;
const int BINDLESS_BUFFER_COUNT		= 128;

const size_t LINE_STRIDE = sizeof(Vector4) + sizeof(Vector4);
const size_t TEXT_STRIDE = sizeof(Vector2) + sizeof(Vector2) + sizeof(Vector4);

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

	skyboxTexture = (AGCTexture*)LoadTexture("Skybox.dds");

	unitQuad = Mesh::Quad<AGCMesh>(1.0f);
	unitQuad->UploadToGPU(this);
	halfUnitQuad = Mesh::Quad<AGCMesh>(0.5f);
	halfUnitQuad->UploadToGPU(this);

	sphere = (AGCMesh*)LoadMesh("Sphere.msh");

	skinningCompute = std::make_unique<AGCShader>("Skinning_c.ags", allocator);
	gammaCompute	= std::make_unique<AGCShader>("Gamma_c.ags", allocator);

	defaultVertexShader = std::make_unique<AGCShader>("Tech_vv.ags", allocator);
	defaultPixelShader  = std::make_unique<AGCShader>("Tech_p.ags"  , allocator);

	uiVertexShader = std::make_unique<AGCShader>("UI_vv.ags", allocator);
	uiPixelShader = std::make_unique<AGCShader>("UI_p.ags", allocator);

	skyboxVertexShader	= std::make_unique<AGCShader>("Skybox_vv.ags", allocator);
	skyboxPixelShader	= std::make_unique<AGCShader>("Skybox_p.ags" , allocator);

	debugLineVertexShader	= std::make_unique<AGCShader>("DebugLine_vv.ags", allocator);
	debugLinePixelShader	= std::make_unique<AGCShader>("DebugLine_p.ags" , allocator);

	debugTextVertexShader	= std::make_unique<AGCShader>("DebugText_vv.ags", allocator);
	debugTextPixelShader	= std::make_unique<AGCShader>("DebugText_p.ags" , allocator);

	deferredVertexShader = std::make_unique<AGCShader>("deferred_vv.ags", allocator);
	deferredPixelShader = std::make_unique<AGCShader>("deferred_p.ags", allocator);

	postVertexShader = std::make_unique<AGCShader>("post_vv.ags", allocator);
	postPixelShader = std::make_unique<AGCShader>("post_p.ags", allocator);

	allFrames = new FrameData[FRAMES_IN_FLIGHT];
	for (int i = 0; i < FRAMES_IN_FLIGHT; ++i) {

		{//We store scene object matrices etc in a big UBO
			allFrames[i].data.dataStart = (char*)allocator.Allocate(1024 * 1024 * 64, sce::Agc::Alignment::kBuffer);
			allFrames[i].data.data = allFrames[i].data.dataStart;

			sce::Agc::Core::BufferSpec bufSpec;
			bufSpec.initAsConstantBuffer(allFrames[i].data.dataStart, sizeof(ShaderConstants));

			checkError(sce::Agc::Core::initialize(&allFrames[i].constantBuffer, &bufSpec));
		}
	}
	currentFrameIndex = 0;
	currentFrame = &allFrames[currentFrameIndex];

	Debug::CreateDebugFont("PressStart2P.fnt", *LoadTexture("PressStart2P.png"));

	sceneBuffer = createBuffer("Scene", FrameBuffer::Slot::Color);
	sceneNormalBuffer = createBuffer("SceneNormal", FrameBuffer::Slot::Normal);
	lightDiffuse = createBuffer("LightsDiffuse", FrameBuffer::Slot::Color);
	lightSpecular = createBuffer("LightsSpecular", FrameBuffer::Slot::Specular);
	screenBuffer = createBuffer("Screen", FrameBuffer::Slot::Color);
}

GameTechAGCRenderer::~GameTechAGCRenderer()	{
	delete sphere;
}

Mesh* GameTechAGCRenderer::LoadMesh(const std::string& name) {
	AGCMesh* m = new AGCMesh();
	MshLoader::LoadMesh(name, *m);
	m->UploadToGPU(this);
	return m;
}

NCL::PS5::AGCTexture* GameTechAGCRenderer::CreateFrameBufferTextureSlot(const std::string& name) {
	uint32_t index = textureMap.size();
	auto t = std::make_unique<AGCTexture>(allocator);
	t->SetAssetID(index);
	bindlessTextures[t->GetAssetID()] = *t->GetAGCPointer();

	auto ptr = t.get();
	textureMap.insert({ name, std::move(t) });
	return ptr;
}

Texture* GameTechAGCRenderer::LoadTexture(const std::string& name) {
	auto found = textureMap.find(name);
	if (found != textureMap.end()) {
		return (Texture*)found->second.get();
	}
	auto t = std::make_unique<AGCTexture>(name, allocator);
	t->SetAssetID(textureMap.size());
	bindlessTextures[t->GetAssetID()] = *t->GetAGCPointer();

	auto ptr = t.get();
	textureMap.insert({name, std::move(t)});

	return ptr;
}

void GameTechAGCRenderer::RenderFrame() {
	currentFrame = &allFrames[currentFrameIndex];

	currentFrame->data.Reset();

	currentFrame->globalDataOffset	= 0;
	currentFrame->debugTextOffset	= 0;
	currentFrame->textVertCount		= 0;
	currentFrame->lineVertCount		= 0;

	//Step 1: Write the frame's constant data to the buffer
	WriteRenderPassConstants();
	//Step 2: Walk the object list and build up the object set and required buffer memory
	UpdateObjectList();
	//Step 4: Go through the geometry and darw it to a shadow map
	//ShadowmapPass();
	//Step 5: Draw a skybox to our main scene render target
	SkyboxPass();
	//Step 6: Draw the scene to our main scene render target
	MainRenderPass();

	// Step 7: Apply post processing to the scene buffer
	LightPass();
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

	frameData.viewProjMatrix = frameData.projMatrix * frameData.viewMatrix;

	frameData.inverseViewProjMatrix = Matrix::Inverse(frameData.viewProjMatrix);
	frameData.inverseViewMatrix = Matrix::Inverse(frameData.viewMatrix);
	frameData.inverseProjMatrix = Matrix::Inverse(frameData.projMatrix);

	frameData.orthoMatrix = Matrix::Orthographic(0.0f, 100.0f, 100.0f, 0.0f, -1.0f, 1.0f);

	frameData.vingetteSettings.enabled = GetVignetteOn();
	frameData.vingetteSettings.color = vignetteColour;
	frameData.vingetteSettings.intensity = vignetteIntensity;
	frameData.vingetteSettings.pulse = vignettePulse;

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

	for (int i = 0; i < frameObjects.size(); ++i) {
		AGCMesh* objectMesh = (AGCMesh*)frameObjects[i]->GetMesh();

	//The new mesh is different than previous meshes, flush out the old list
		if( prevMesh != objectMesh) {
			prevMesh->BindVertexBuffers(frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs));

			uint32_t* objID = static_cast<uint32_t*>(frameContext->m_dcb.allocateTopDown(sizeof(uint32_t), sce::Agc::Alignment::kBuffer));
			*objID = startingIndex;
			frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs).setUserSrtBuffer(objID, 1);

			DrawBoundMeshInstanced(*frameContext, *prevMesh, instanceCount);
			prevMesh = objectMesh;
			instanceCount = 0;
			startingIndex = i;
		}
		if (i == frameObjects.size() - 1) {
			objectMesh->BindVertexBuffers(frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs));

			uint32_t* objID = static_cast<uint32_t*>(frameContext->m_dcb.allocateTopDown(sizeof(uint32_t), sce::Agc::Alignment::kBuffer));
			*objID = startingIndex;
			frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs).setUserSrtBuffer(objID, 1);

			if (prevMesh == objectMesh) {
				instanceCount++;
			}

			DrawBoundMeshInstanced(*frameContext, *objectMesh, instanceCount);
		}
		else {
			instanceCount++;
		}
	}
}

void GameTechAGCRenderer::SkyboxPass() {
	frameContext->setShaders(nullptr, skyboxVertexShader->GetAGCPointer(), skyboxPixelShader->GetAGCPointer(), sce::Agc::UcPrimitiveType::Type::kTriList);
	useViewPort(frameContext, ScreenSize);

	sce::Agc::CxRenderTargetMask rtMask = sce::Agc::CxRenderTargetMask().init().setMask(0, 0xFF);
	frameContext->m_sb.setState(rtMask);
	frameContext->m_sb.setState(sceneBuffer.target);

	frameContext->m_sb.setState(depthTarget);

	sce::Agc::CxDepthStencilControl depthControl;
	depthControl.init();
	depthControl.setDepth(sce::Agc::CxDepthStencilControl::Depth::kDisable);
	depthControl.setDepthFunction(sce::Agc::CxDepthStencilControl::DepthFunction::kAlways);
	depthControl.setDepthWrite(sce::Agc::CxDepthStencilControl::DepthWrite::kDisable);
	frameContext->m_sb.setState(depthControl);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kPs)
		.setSamplers(0, 1, &defaultSampler)
		.setTextures(1, 1, skyboxTexture->GetAGCPointer());

	unitQuad->BindVertexBuffers(frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs));
	DrawBoundMesh(*frameContext, *unitQuad);
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

void GameTechAGCRenderer::PostProcessPass()
{
	frameContext->setShaders(nullptr, postVertexShader->GetAGCPointer(), postPixelShader->GetAGCPointer(), sce::Agc::UcPrimitiveType::Type::kTriList);
	useViewPort(frameContext, ScreenSize);

	sce::Agc::CxRenderTargetMask rtMask = sce::Agc::CxRenderTargetMask().init().setMask(0, 0xFF);
	frameContext->m_sb.setState(rtMask);
	frameContext->m_sb.setState(screenBuffer.target);

	sce::Agc::CxBlendControl blendControl;
	blendControl.init().setBlend(sce::Agc::CxBlendControl::Blend::kDisable);
	frameContext->m_sb.setState(blendControl);

	frameContext->m_sb.setState(sce::Agc::CxPrimitiveSetup().init().setCullFace(sce::Agc::CxPrimitiveSetup::CullFace::kBack));

	sce::Agc::CxDepthStencilControl depthControl;
	depthControl.init();
	depthControl.setDepth(sce::Agc::CxDepthStencilControl::Depth::kDisable);
	depthControl.setDepthFunction(sce::Agc::CxDepthStencilControl::DepthFunction::kAlways);
	depthControl.setDepthWrite(sce::Agc::CxDepthStencilControl::DepthWrite::kDisable);
	frameContext->m_sb.setState(depthControl);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kPs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setSamplers(0, 1, &sceneBuffer.sampler).setTextures(0, 1, sceneBuffer.texture->GetAGCPointer())
		.setSamplers(1, 1, &lightDiffuse.sampler).setTextures(1, 1, lightDiffuse.texture->GetAGCPointer())
		.setSamplers(2, 1, &lightSpecular.sampler).setTextures(2, 1, lightSpecular.texture->GetAGCPointer());

	unitQuad->BindVertexBuffers(frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs));
	DrawBoundMesh(*frameContext, *unitQuad);
}

void GameTechAGCRenderer::UpdateDebugData() {
	const std::vector<NCL::Debug::DebugStringEntry>& strings = NCL::Debug::GetDebugStrings();
	const std::vector<Debug::DebugLineEntry>& lines = Debug::GetDebugLines();



	for (const auto& s : strings) {
		currentFrame->textVertCount += Debug::GetDebugFont()->GetVertexCountForString(s.data);
	}
	currentFrame->lineVertCount = (int)lines.size() * 2;

	currentFrame->data.WriteData((void*)lines.data(), (size_t)currentFrame->lineVertCount * LINE_STRIDE);

	currentFrame->debugTextOffset = currentFrame->data.bytesWritten;
	std::vector< NCL::Rendering::SimpleFont::InterleavedTextVertex> verts;

	for (const auto& s : strings) {
		float size = 20.0f;
		Debug::GetDebugFont()->BuildInterleavedVerticesForString(s.data, s.position, s.colour, size, verts);
		//can now copy to GPU visible mem
		size_t count = verts.size() * TEXT_STRIDE;
		memcpy(currentFrame->data.data, verts.data(), count);
		currentFrame->data.data += count;
		currentFrame->data.bytesWritten += count;
		verts.clear();
	}
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
	if (currentFrame->lineVertCount == 0) {
		return;
	}
	frameContext->setShaders(nullptr, debugLineVertexShader->GetAGCPointer(), debugLinePixelShader->GetAGCPointer(), sce::Agc::UcPrimitiveType::Type::kLineList);
	sce::Agc::CxDepthStencilControl depthControl;
	depthControl.init()
		.setDepth(sce::Agc::CxDepthStencilControl::Depth::kDisable)
		.setDepthWrite(sce::Agc::CxDepthStencilControl::DepthWrite::kDisable);
	frameContext->m_sb.setState(depthControl);

	char* dataPos		= currentFrame->data.dataStart + currentFrame->debugLinesOffset;
	size_t dataCount	= currentFrame->lineVertCount;

	sce::Agc::Core::BufferSpec bufSpec;
	bufSpec.initAsRegularBuffer(dataPos, LINE_STRIDE, dataCount);
	checkError(sce::Agc::Core::initialize(&currentFrame->debugLineBuffer, &bufSpec));

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setBuffers(0, 1, &currentFrame->debugLineBuffer);

	frameContext->drawIndexAuto(currentFrame->lineVertCount);
}

void GameTechAGCRenderer::RenderDebugText() {
	if (currentFrame->textVertCount == 0) {
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

	char* dataPos = currentFrame->data.dataStart + currentFrame->debugTextOffset;
	size_t dataCount = currentFrame->textVertCount;

	sce::Agc::Core::BufferSpec bufSpec;
	bufSpec.initAsRegularBuffer(dataPos, TEXT_STRIDE, dataCount);
	checkError(sce::Agc::Core::initialize(&currentFrame->debugTextBuffer, &bufSpec));

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setBuffers(0, 1, &currentFrame->debugTextBuffer);

	AGCTexture* debugTex = (AGCTexture*)Debug::GetDebugFont()->GetTexture();

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kPs)
		.setSamplers(0, 1, &pixelSampler)
		.setTextures(1, 1, debugTex->GetAGCPointer());

	frameContext->drawIndexAuto(currentFrame->textVertCount);
}

void GameTechAGCRenderer::UpdateObjectList() {
	currentFrame->objects.begin(currentFrame);
	for (auto g : frameObjects) {
		ObjectState state;
		Matrix4 transMatrix;
		g->getParent()->GetTransform().getOpenGLMatrix((float*)&transMatrix);
		state.modelMatrix = transMatrix * Matrix::Scale(g->getParent()->getRenderScale());
		// Matrix without translation components, for normal transforms
		Matrix3 transMat3 = Matrix::FromMat4(transMatrix);
		state.normalMatrix = Matrix::FromMat3(Matrix::InverseTranspose(transMat3));

		state.colour = g->GetColour();
		state.texRepeats = g->GetTexRepeating();
		state.texScale = g->getParent()->getRenderScale() * g->GetTexScaleMultiplier();
		state.skinningIndex = NULLTEX;

		Texture* t = g->GetDefaultTexture();
		state.texIndex = t ? t->GetAssetID() : NULLTEX;

		Texture* normal = g->GetNormalMap();
		state.normalIndex = normal ? normal->GetAssetID() : NULLTEX;

		AGCMesh* m = (AGCMesh*)g->GetMesh();
		if (m && m->GetJointCount() > 0) {//It's a skeleton mesh, need to update transformed vertices buffer

			Buffer* b = g->GetGPUBuffer();
			if (!b) {
				//We've not yet made a buffer to hold the verts of this mesh!
				//We need a new mesh to store the positions, normals, and tangents of this mesh
				size_t vertexSize = sizeof(Vector3) + sizeof(Vector3) + sizeof(Vector4);
				size_t vertexCount = m->GetVertexCount();
				size_t bufferSize = vertexCount * vertexSize;

				char* vertexData = (char*)allocator.Allocate((uint64_t)(bufferSize), sce::Agc::Alignment::kBuffer);

				sce::Agc::Core::BufferSpec bufSpec;
				bufSpec.initAsRegularBuffer(vertexData, vertexSize, vertexCount);

				sce::Agc::Core::Buffer vBuffer;
				checkError(sce::Agc::Core::initialize(&vBuffer, &bufSpec));

				uint32_t bufferID = bufferCount++;
				b = new AGCBuffer(vBuffer, vertexData);
				b->SetAssetID(bufferID);
				g->SetGPUBuffer(b);

				bindlessBuffers[bufferID] = vBuffer;
			}
			state.skinningIndex = b->GetAssetID();

			frameJobs.push_back({ g, b->GetAssetID() });
		}
		currentFrame->data.WriteData<ObjectState>(state);
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
	auto elemsWritten = bytesWritten / sizeof(T);
	spec.initAsRegularBuffer(start, sizeof(T), elemsWritten);
	sce::Agc::Core::initialize(&buffer, &spec);
}
