#include "GameTechRenderer.h"
#include "GameObject.h"
#include "RenderObject.h"
#include "Camera.h"
#include "TextureLoader.h"
#include "MshLoader.h"
#include "ResourceManager.h"
#include "PointLight.h"

#include "Debug.h"

#include <NCLCoreClasses/stb/stb_image.h>

using namespace NCL;
using namespace Rendering;
using namespace CSC8503;

#define SHADOWSIZE 4096

Matrix4 biasMatrix = Matrix::Translation(Vector3(0.5f, 0.5f, 0.5f)) * Matrix::Scale(Vector3(0.5f, 0.5f, 0.5f));

GameTechRenderer::GameTechRenderer(Window* window) : OGLRenderer(window), GameTechRendererInterface(window) {
	glEnable(GL_DEPTH_TEST);

	debugShader = std::make_unique<OGLShader>("Debug.vert", "Debug.frag");
	shadowShader = std::make_unique<OGLShader>("shadow.vert", "shadow.frag");
	sceneShader = std::make_unique<OGLShader>("scene.vert", "deferredscenefrag.glsl");
	decalShader = std::make_unique<OGLShader>("decal.vert", "decal.frag");
	decalBlendShader = std::make_unique<OGLShader>("texturevert.glsl", "decalBlend.frag");
	uiShader = std::make_unique<OGLShader>("ui.vert", "ui.frag");

	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
			     SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(1, 1, 1, 1);

	//Set up the light properties
	lightColour = Vector4(0.8f, 0.8f, 0.5f, 1.0f);
	lightRadius = 1000.0f;
	lightPosition = Vector3(-200.0f, 60.0f, -200.0f);
	//Skybox!
	skyboxShader = std::make_unique<OGLShader>("skybox.vert", "skybox.frag");
	skyboxMesh = std::make_unique<OGLMesh>();
	skyboxMesh->SetVertexPositions({Vector3(-1, 1,-1), Vector3(-1,-1,-1) , Vector3(1,-1,-1) , Vector3(1,1,-1) });
	skyboxMesh->SetVertexIndices({ 0,1,2,2,3,0 });
	skyboxMesh->UploadToGPU();

	LoadSkybox();

	glGenVertexArrays(1, &lineVAO);
	glGenVertexArrays(1, &textVAO);

	glGenBuffers(1, &lineVertVBO);
	glGenBuffers(1, &textVertVBO);
	glGenBuffers(1, &textColourVBO);
	glGenBuffers(1, &textTexVBO);

	Debug::CreateDebugFont("PressStart2P.fnt", *LoadTexture("PressStart2P.png"));

	unitQuad = Mesh::Quad<OGLMesh>(1.0f);
	unitQuad->UploadToGPU();
	halfUnitQuad = Mesh::Quad<OGLMesh>(0.5f);
	halfUnitQuad->UploadToGPU();

	SetDebugStringBufferSizes(10000);
	SetDebugLineBufferSizes(1000);

	/////////InitCrosshair(); //This line Ameya added for crosshair THINK THIS CAN BE REMOVED, NOT SURE YET IF INITUI REPLACES IT ANYWHERE

	//Deferred rendering additions:
	deferredsceneShader = new OGLShader("scene.vert", "deferredscenefrag.glsl");
	pointlightShader = new OGLShader("pointlightvertex.glsl", "pointlightfrag.glsl");
	combineShader = new OGLShader("texturevert.glsl", "combinefrag.glsl");

	lightSphere = new OGLMesh(); 
	lightSphere = LoadMesh("Sphere.msh"); //Load mesh takes care of upload to GPU itself

	glGenFramebuffers(1, &bufferFBO);
	glGenFramebuffers(1, &pointLightFBO);

	GLenum buffers[2] = {
	GL_COLOR_ATTACHMENT0,
	GL_COLOR_ATTACHMENT1 
	};

	GenerateScreenTexture(bufferDepthTex, true);
	GenerateScreenTexture(bufferColourTex);
	GenerateScreenTexture(bufferNormalTex, false); 

	GenerateScreenTexture(lightDiffuseTex);
	GenerateScreenTexture(lightSpecularTex);
	//attach textures to FBOS:
	//first pass:
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bufferNormalTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glDrawBuffers(2, buffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !bufferColourTex || !bufferNormalTex || !bufferDepthTex ) {//check attachment success
		return;
	}

	//second pass:
	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightDiffuseTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, lightSpecularTex, 0);
	glDrawBuffers(2, buffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !lightDiffuseTex || ! lightSpecularTex) {
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//not enabling depth test etc here as this is done in the rendering functions

	//Post processing additions: 
	hdrShader = new OGLShader("texturevert.glsl", "hdrfrag.glsl");
	
	fullscreenQuad = new OGLMesh();
	fullscreenQuad->SetVertexPositions({ Vector3(-1, 1,0), Vector3(-1,-1,0) , Vector3(1,-1,0) , Vector3(1,1,0) }); 
	fullscreenQuad->SetVertexTextureCoords({ Vector2(0, 1), Vector2(0,0) , Vector2(1,0) , Vector2(1,1) }); 
	fullscreenQuad->SetVertexIndices({ 0,1,2,2,3,0 }); 
	fullscreenQuad->UploadToGPU(); 

	vignetteShader = new OGLShader("texturevert.glsl", "vignettefrag.glsl");

 	//start setting up framebuffers for post processing:
	//first generate the textures to store the rendered scene:
	glGenTextures(1, &hdrTex); //first, colour attachment
	glBindTexture(GL_TEXTURE_2D, hdrTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowSize.x, windowSize.y, 0, GL_RGBA, GL_FLOAT, NULL); //Floating point texture for HDR.

	glGenTextures(1, &hdrDepthTex); //then, depth-stencil attachment
	glBindTexture(GL_TEXTURE_2D, hdrDepthTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, windowSize.x, windowSize.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrTex, 0); //attach textures to FBO attachments
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, hdrDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, hdrDepthTex, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !hdrTex || !hdrDepthTex) {
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//set up framebuffer for vignette. May rename the framebuffers and textures to be more generic if more effects are added later
	glGenTextures(1, &BTex);
	glBindTexture(GL_TEXTURE_2D, BTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowSize.x, windowSize.y, 0, GL_RGBA, GL_FLOAT, NULL);//making this one floating point as well

	glGenTextures(1, &BDepthTex); //set up depth stencil test
	glBindTexture(GL_TEXTURE_2D, BDepthTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, windowSize.x, windowSize.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);


	glGenFramebuffers(1, &BFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, BFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, BTex, 0); //attach BFBO as the colour attachment
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, BDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, BDepthTex, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !BTex || !BDepthTex) {
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

GameTechRenderer::~GameTechRenderer() {
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);

	delete deferredsceneShader;
	delete pointlightShader;
	delete combineShader;
	delete lightSphere;
	glDeleteFramebuffers(1, &bufferFBO);
	glDeleteFramebuffers(1, &pointLightFBO);
	glDeleteTextures(1, &bufferColourTex);
	glDeleteTextures(1, &bufferNormalTex);
	glDeleteTextures(1, &bufferDepthTex);
	glDeleteTextures(1, &lightDiffuseTex);
	glDeleteTextures(1, &lightSpecularTex);
	glDeleteTextures(1, &hdrTex);
	glDeleteFramebuffers(1, &hdrFBO);
	glDeleteTextures(1, &BTex);
	glDeleteFramebuffers(1, &BFBO);
	delete fullscreenQuad; //only mesh that needs to be deleted as others are std::make_unique<OGLMesh>
	delete hdrShader;
	delete vignetteShader;
}

void GameTechRenderer::LoadSkybox() {
	std::string filenames[6] = {
		"Cubemap/skyrender0004.png",
		"Cubemap/skyrender0001.png",
		"Cubemap/skyrender0003.png",
		"Cubemap/skyrender0006.png",
		"Cubemap/skyrender0002.png",
		"Cubemap/skyrender0005.png"
	};

	uint32_t width[6]	 = { 0 };
	uint32_t height[6]	 = { 0 };
	uint32_t channels[6] = { 0 };
	uint32_t flags[6]		 = { 0 };

	vector<char*> texData(6, nullptr);

	for (int i = 0; i < 6; ++i) {
		TextureLoader::LoadTexture(filenames[i], texData[i], width[i], height[i], channels[i], flags[i]);
		if (i > 0 && (width[i] != width[0] || height[0] != height[0])) {
			std::cout << __FUNCTION__ << " cubemap input textures don't match in size?\n";
			return;
		}
	}
	glGenTextures(1, &skyboxTex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);

	GLenum type = channels[0] == 4 ? GL_RGBA : GL_RGB;

	for (int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width[i], height[i], 0, type, GL_UNSIGNED_BYTE, texData[i]);
		stbi_image_free(texData[i]);
	}

	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void GameTechRenderer::RenderFrame() {
	/*glEnable(GL_CULL_FACE); //this line and one below are included in draw scene. Only thing missing is shadows
	glClearColor(1, 1, 1, 1);
	RenderShadowMap();*/
	//Set up to render into framebuffer
	//THE ABOVE FEW LINES HAVE BEEN LEFT TO SHOW HOW SHADOWS WERE ORIGINALLY HANDLED

	//////deferred rendering version:
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	//RenderShadowMap();
	FillBuffers();
	DrawPointLights();
	CombineBuffers();
	RenderPostProcessing(); 
	RenderUI(); //Call this function when in game, this renders crosshair as of now

	glDisable(GL_CULL_FACE); //Todo - text indices are going the wrong way...
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	NewRenderLines();
	NewRenderTextures();
	NewRenderText();
}

void GameTechRenderer::RenderShadowMap() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);

	glCullFace(GL_FRONT);

	UseShader(*shadowShader);
	int mvpLocation = glGetUniformLocation(shadowShader->GetProgramID(), "mvpMatrix");

	Matrix4 shadowViewMatrix = Matrix::View(lightPosition, Vector3(0, 0, 0), Vector3(0,1,0));
	Matrix4 shadowProjMatrix = Matrix::Perspective(100.0f, 500.0f, 1.0f, 45.0f);

	Matrix4 mvMatrix = shadowProjMatrix * shadowViewMatrix;

	shadowMatrix = biasMatrix * mvMatrix; //we'll use this one later on

	for (const auto&i : frameObjects) {
		//Matrix4 modelMatrix = (*i).getParent()->GetTransform().getOpenGLMatrix ()->GetMatrix();
		Matrix4 modelMatrix;
		i->getParent()->GetTransform().getOpenGLMatrix((btScalar*)&modelMatrix);
		modelMatrix = modelMatrix * Matrix::Scale(i->getParent()->getRenderScale());
		Matrix4 mvpMatrix	= mvMatrix * modelMatrix;
		glUniformMatrix4fv(mvpLocation, 1, false, (float*)&mvpMatrix);
		BindMesh((OGLMesh&)*(*i).GetMesh());
		size_t layerCount = (*i).GetMesh()->GetSubMeshCount();
		for (size_t i = 0; i < layerCount; ++i) {
			DrawBoundMesh((uint32_t)i);
		}
	}

	glViewport(0, 0, windowSize.x, windowSize.y);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glCullFace(GL_BACK);
}

void GameTechRenderer::RenderSkybox() {
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	Matrix4 viewMatrix = camera->BuildViewMatrix();
	Matrix4 projMatrix = camera->BuildProjectionMatrix(hostWindow->GetScreenAspect());

	UseShader(*skyboxShader);

	int projLocation = glGetUniformLocation(skyboxShader->GetProgramID(), "projMatrix");
	int viewLocation = glGetUniformLocation(skyboxShader->GetProgramID(), "viewMatrix");
	int texLocation  = glGetUniformLocation(skyboxShader->GetProgramID(), "cubeTex");

	glUniformMatrix4fv(projLocation, 1, false, (float*)&projMatrix);
	glUniformMatrix4fv(viewLocation, 1, false, (float*)&viewMatrix);

	glUniform1i(texLocation, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);

	BindMesh(*skyboxMesh);
	DrawBoundMesh();

	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void GameTechRenderer::RenderCamera() {
	Matrix4 viewMatrix = camera->BuildViewMatrix();
	Matrix4 projMatrix = camera->BuildProjectionMatrix(hostWindow->GetScreenAspect());

	UseShader(*deferredsceneShader); //changed all sceneShader to deferredsceneShader
	int projLocation	= glGetUniformLocation(deferredsceneShader->GetProgramID(), "projMatrix");
	int viewLocation	= glGetUniformLocation(deferredsceneShader->GetProgramID(), "viewMatrix");
	int modelLocation	= glGetUniformLocation(deferredsceneShader->GetProgramID(), "modelMatrix");
	int colourLocation  = glGetUniformLocation(deferredsceneShader->GetProgramID(), "objectColour");
	int hasVColLocation = glGetUniformLocation(deferredsceneShader->GetProgramID(), "hasVertexColours");
	int hasTexLocation  = glGetUniformLocation(deferredsceneShader->GetProgramID(), "hasTexture");
	int shadowLocation  = glGetUniformLocation(deferredsceneShader->GetProgramID(), "shadowMatrix");
	int hasFlatLocation = glGetUniformLocation(deferredsceneShader->GetProgramID(), "isFlat");
	int hasNormalLocation = glGetUniformLocation(deferredsceneShader->GetProgramID(), "hasNormalMap");
	int texRepeatingLocation = glGetUniformLocation(deferredsceneShader->GetProgramID(), "texRepeating");
	int texScaleLocation = glGetUniformLocation(deferredsceneShader->GetProgramID(), "texScale");

	/*
	int lightPosLocation	= glGetUniformLocation(sceneShader->GetProgramID(), "lightPos"); Lighting to be deferred
	int lightColourLocation = glGetUniformLocation(sceneShader->GetProgramID(), "lightColour");
	int lightRadiusLocation = glGetUniformLocation(sceneShader->GetProgramID(), "lightRadius");
	*/

	int cameraLocation = glGetUniformLocation(deferredsceneShader->GetProgramID(), "cameraPos"); //changing this to deferredsceneShader

	Vector3 camPos = camera->GetPosition();
	glUniform3fv(cameraLocation, 1, &camPos.x);

	glUniformMatrix4fv(projLocation, 1, false, (float*)&projMatrix);
	glUniformMatrix4fv(viewLocation, 1, false, (float*)&viewMatrix);

	/*
	glUniform3fv(lightPosLocation, 1, (float*)&lightPosition);
	glUniform4fv(lightColourLocation, 1, (float*)&lightColour);
	glUniform1f(lightRadiusLocation, lightRadius);*/

	//int shadowTexLocation = glGetUniformLocation(sceneShader->GetProgramID(), "shadowTex"); Also temporarily removing shadows
	//glUniform1i(shadowTexLocation, 1);

	//TODO - PUT IN FUNCTION
	//glActiveTexture(GL_TEXTURE0 + 1); TEMPORARILY REMOVED THESE TWO LINES
	//glBindTexture(GL_TEXTURE_2D, shadowTex);

	for (const auto&i : frameObjects) {

		if ((*i).GetDefaultTexture()) { 
			BindTextureToShader(*(OGLTexture*)(*i).GetDefaultTexture(), "diffuseTex", 0); //was maintTex for scenefrag. Using diffuseTex for deferredscenefrag
			//figure out scale of object:
			Vector3 scale = i->getParent()->getRenderScale() * i->GetTexScaleMultiplier();
			glUniform3fv(texScaleLocation, 1, scale.array);

		}

		//normal map capabilities added:
		if ((*i).GetNormalMap()) { //changed texture unit to 1 instead of 2 while there are no shadows
			BindTextureToShader(*(OGLTexture*)(*i).GetNormalMap(), "normalTex", 1); //need a shader that utilises normal maps, has a uniform sampler2D called "normalTex" in texture unit 2
		}

		//Matrix4 modelMatrix = (*i).GetTransform()->GetMatrix();
		Matrix4 modelMatrix;
		i->getParent()->GetTransform().getOpenGLMatrix((btScalar*)&modelMatrix);
		modelMatrix = modelMatrix * Matrix::Scale(i->getParent()->getRenderScale());
		glUniformMatrix4fv(modelLocation, 1, false, (float*)&modelMatrix);

		//Matrix4 fullShadowMat = shadowMatrix * modelMatrix; //TEMPORARILY REMOVING SHADOWS
		//glUniformMatrix4fv(shadowLocation, 1, false, (float*)&fullShadowMat);

		Vector4 colour = i->GetColour();
		glUniform4fv(colourLocation, 1, &colour.x);

		glUniform1i(hasVColLocation, !(*i).GetMesh()->GetColourData().empty());

		glUniform1i(hasTexLocation, (OGLTexture*)(*i).GetDefaultTexture() ? 1:0);
		glUniform1i(hasFlatLocation, i->GetIsFlat());
		glUniform1i(hasNormalLocation, i->GetHasNormal());
		glUniform1i(texRepeatingLocation, i->GetTexRepeating());

		BindMesh((OGLMesh&)*(*i).GetMesh());
		size_t layerCount = (*i).GetMesh()->GetSubMeshCount();
		for (size_t i = 0; i < layerCount; ++i) {
			DrawBoundMesh((uint32_t)i);
		}
	}
}

OGLMesh* GameTechRenderer::LoadMesh(const std::string& name) {///changed this to return OGLMesh* instead of Mesh*
	OGLMesh* mesh = new OGLMesh();
	MshLoader::LoadMesh(name, *mesh);
	mesh->SetPrimitiveType(GeometryPrimitive::Triangles);
	mesh->UploadToGPU();
	return mesh;
}

void GameTechRenderer::NewRenderLines() {
	const std::vector<Debug::DebugLineEntry>& lines = Debug::GetDebugLines();
	if (lines.empty()) {
		return;
	}

	Matrix4 viewMatrix = camera->BuildViewMatrix();
	Matrix4 projMatrix = camera->BuildProjectionMatrix(hostWindow->GetScreenAspect());

	Matrix4 viewProj  = projMatrix * viewMatrix;

	UseShader(*debugShader);
	int matSlot = glGetUniformLocation(debugShader->GetProgramID(), "viewProjMatrix");
	GLuint texSlot = glGetUniformLocation(debugShader->GetProgramID(), "useTexture");
	glUniform1i(texSlot, 0);

	glUniformMatrix4fv(matSlot, 1, false, (float*)viewProj.array);

	debugLineData.clear();

	size_t frameLineCount = lines.size() * 2;

	SetDebugLineBufferSizes(frameLineCount);

	glBindBuffer(GL_ARRAY_BUFFER, lineVertVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, lines.size() * sizeof(Debug::DebugLineEntry), lines.data());


	glBindVertexArray(lineVAO);
	glDrawArrays(GL_LINES, 0, (GLsizei)frameLineCount);
	glBindVertexArray(0);
}

void GameTechRenderer::NewRenderText() {
	const std::vector<Debug::DebugStringEntry>& strings = Debug::GetDebugStrings();
	if (strings.empty()) {
		return;
	}

	UseShader(*debugShader);

	OGLTexture* t = (OGLTexture*)Debug::GetDebugFont()->GetTexture();

	if (t) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, t->GetObjectID());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
		BindTextureToShader(*t, "mainTex", 0);
	}

	Matrix4 proj = Matrix::Orthographic(0.0f, 100.0f, 100.0f, 0.0f, -1.0f, 1.0f);

	int matSlot = glGetUniformLocation(debugShader->GetProgramID(), "viewProjMatrix");
	glUniformMatrix4fv(matSlot, 1, false, (float*)proj.array);

	GLuint texSlot = glGetUniformLocation(debugShader->GetProgramID(), "useTexture");
	glUniform1i(texSlot, 1);

	debugTextPos.clear();
	debugTextColours.clear();
	debugTextUVs.clear();

	int frameVertCount = 0;
	for (const auto& s : strings) {
		frameVertCount += Debug::GetDebugFont()->GetVertexCountForString(s.data);
	}
	SetDebugStringBufferSizes(frameVertCount);

	for (const auto& s : strings) {
		float size = 20.0f;
		Debug::GetDebugFont()->BuildVerticesForString(s.data, s.position, s.colour, size, debugTextPos, debugTextUVs, debugTextColours);
	}

	glBindBuffer(GL_ARRAY_BUFFER, textVertVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, frameVertCount * sizeof(Vector3), debugTextPos.data());
	glBindBuffer(GL_ARRAY_BUFFER, textColourVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, frameVertCount * sizeof(Vector4), debugTextColours.data());
	glBindBuffer(GL_ARRAY_BUFFER, textTexVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, frameVertCount * sizeof(Vector2), debugTextUVs.data());

	glBindVertexArray(textVAO);
	glDrawArrays(GL_TRIANGLES, 0, frameVertCount);
	glBindVertexArray(0);
}

void GameTechRenderer::NewRenderTextures() {
	const std::vector<Debug::DebugTexEntry>& texEntries = Debug::GetDebugTex();
	if (texEntries.empty()) {
		return;
	}
	UseShader(*debugShader);

	Matrix4 proj = Matrix::Orthographic(0.0f, 100.0f, 100.0f, 0.0f, -1.0f, 1.0f);

	int matSlot = glGetUniformLocation(debugShader->GetProgramID(), "viewProjMatrix");
	glUniformMatrix4fv(matSlot, 1, false, (float*)proj.array);

	GLuint texSlot = glGetUniformLocation(debugShader->GetProgramID(), "useTexture");
	glUniform1i(texSlot, 2);

	GLuint useColourSlot = glGetUniformLocation(debugShader->GetProgramID(), "useColour");
	glUniform1i(useColourSlot, 1);

	GLuint colourSlot = glGetUniformLocation(debugShader->GetProgramID(), "texColour");

	BindMesh(*unitQuad);

	glActiveTexture(GL_TEXTURE0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	for (const auto& tex : texEntries) {
		OGLTexture* t = (OGLTexture*)tex.t;
		glBindTexture(GL_TEXTURE_2D, t->GetObjectID());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		BindTextureToShader(*t, "mainTex", 0);

		Matrix4 transform = Matrix::Translation(Vector3(tex.position.x, tex.position.y, 0)) * Matrix::Scale(Vector3(tex.scale.x, tex.scale.y, 1.0f));
		Matrix4 finalMatrix = proj * transform;

		glUniformMatrix4fv(matSlot, 1, false, (float*)finalMatrix.array);

		glUniform4f(colourSlot, tex.colour.x, tex.colour.y, tex.colour.z, tex.colour.w);

		DrawBoundMesh();
	}

	glUniform1i(useColourSlot, 0);
}

Texture* GameTechRenderer::LoadTexture(const std::string& name) {
	return OGLTexture::TextureFromFile(name).release();
}

void GameTechRenderer::SetDebugStringBufferSizes(size_t newVertCount) {
	if (newVertCount > textCount) {
		textCount = newVertCount;

		glBindBuffer(GL_ARRAY_BUFFER, textVertVBO);
		glBufferData(GL_ARRAY_BUFFER, textCount * sizeof(Vector3), nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, textColourVBO);
		glBufferData(GL_ARRAY_BUFFER, textCount * sizeof(Vector4), nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, textTexVBO);
		glBufferData(GL_ARRAY_BUFFER, textCount * sizeof(Vector2), nullptr, GL_DYNAMIC_DRAW);

		debugTextPos.reserve(textCount);
		debugTextColours.reserve(textCount);
		debugTextUVs.reserve(textCount);

		glBindVertexArray(textVAO);

		glVertexAttribFormat(0, 3, GL_FLOAT, false, 0);
		glVertexAttribBinding(0, 0);
		glBindVertexBuffer(0, textVertVBO, 0, sizeof(Vector3));

		glVertexAttribFormat(1, 4, GL_FLOAT, false, 0);
		glVertexAttribBinding(1, 1);
		glBindVertexBuffer(1, textColourVBO, 0, sizeof(Vector4));

		glVertexAttribFormat(2, 2, GL_FLOAT, false, 0);
		glVertexAttribBinding(2, 2);
		glBindVertexBuffer(2, textTexVBO, 0, sizeof(Vector2));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
	}
}

void GameTechRenderer::SetDebugLineBufferSizes(size_t newVertCount) {
	if (newVertCount > lineCount) {
		lineCount = newVertCount;

		glBindBuffer(GL_ARRAY_BUFFER, lineVertVBO);
		glBufferData(GL_ARRAY_BUFFER, lineCount * sizeof(Debug::DebugLineEntry), nullptr, GL_DYNAMIC_DRAW);

		debugLineData.reserve(lineCount);

		glBindVertexArray(lineVAO);

		int realStride = sizeof(Debug::DebugLineEntry) / 2;

		glVertexAttribFormat(0, 3, GL_FLOAT, false, offsetof(Debug::DebugLineEntry, start));
		glVertexAttribBinding(0, 0);
		glBindVertexBuffer(0, lineVertVBO, 0, realStride);

		glVertexAttribFormat(1, 4, GL_FLOAT, false, offsetof(Debug::DebugLineEntry, colourA));
		glVertexAttribBinding(1, 0);
		glBindVertexBuffer(1, lineVertVBO, sizeof(Vector4), realStride);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
	}
}

void GameTechRenderer::RenderUI() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
	//RenderPostProcessing();
	UseShader(*uiShader);

	// Get uniform locations once and store them
	int positionLocation = glGetUniformLocation(uiShader->GetProgramID(), "position");
	int sizeLocation = glGetUniformLocation(uiShader->GetProgramID(), "size");
	int colorLocation = glGetUniformLocation(uiShader->GetProgramID(), "color");
	int hasTextureLocation = glGetUniformLocation(uiShader->GetProgramID(), "hasTexture");
	int textureLocation = glGetUniformLocation(uiShader->GetProgramID(), "mainTex");


	for (const auto& uiElement : frameSprites) {
		Vector2 pos = uiElement.position;
		Vector2 size = uiElement.size;
		Vector4 color = uiElement.color;

		// Set uniform values
		glUniform2fv(positionLocation, 1, (float*)&pos);
		glUniform2fv(sizeLocation, 1, (float*)&size);
		glUniform4fv(colorLocation, 1, (float*)&color);

		// Handle textures
		if (uiElement.texture) {
			glUniform1i(hasTextureLocation, 1);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ((OGLTexture*)uiElement.texture.get())->GetObjectID());
			glUniform1i(textureLocation, 0);
		}
		else {
			glUniform1i(hasTextureLocation, 0);
		}

		// Render UI quad
		BindMesh(*halfUnitQuad);
		DrawBoundMesh();
	}

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

/*
	RenderQuad() is a helper function that renders a quad to the screen.
	The first time this function is called, it will generate the VAO and VBO with the quad vertices.
	Subsequent calls will simply bind the VAO and draw the quad.
*/
void GameTechRenderer::RenderQuad() {
	// Generate the VAO and VBO for the quad if it doesn't already exist
	if (decalQuadVAO == 0) {
		float quadVertices[] = {
			// Positions        // Texture Coords
			-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
			 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,

			-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
			 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
			 1.0f,  1.0f, 0.0f,  1.0f, 1.0f
		};

		// Generate and bind VAO and VBO
		glGenVertexArrays(1, &decalQuadVAO);
		glGenBuffers(1, &decalQuadVBO);
		glBindVertexArray(decalQuadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, decalQuadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

		// Set up position attribute
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

		// Set up texture coordinate attribute
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}

	// Bind the VAO and draw the quad
	glBindVertexArray(decalQuadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void GameTechRenderer::RenderDecals() {
	// Bind the decal FBO to keep decal rendering separate from the main scene
	glBindFramebuffer(GL_FRAMEBUFFER, decalSystem.GetDecalFBO());

	// Copy the scene depth buffer to the decal FBO
	glBindFramebuffer(GL_READ_FRAMEBUFFER, bufferFBO); //was hdrFBO. Depth can now come from bufferFBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, decalSystem.GetDecalFBO());
	glBlitFramebuffer(0, 0, windowSize.x, windowSize.y, // Read buffer (scene)
					  0, 0, windowSize.x, windowSize.y, // Write buffer (decals)
					  GL_DEPTH_BUFFER_BIT, GL_NEAREST); // Copy depth buffer

	// Rebinding the FBO is necessary because the depth buffer is copied from the scene FBO
	glBindFramebuffer(GL_FRAMEBUFFER, decalSystem.GetDecalFBO());

	glClearColor(0, 0, 0, 0); //Clearing here with a = 1 removes all colour and textures. Not clearing also removes all textures. Not really sure what this does
	// Not clearing the depth buffer because it was copied from the scene FBO
	// This is to ensure that the decals are projected onto the correct surfaces
	glClear(GL_COLOR_BUFFER_BIT); // clear the decal FBO

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST); // Enable depth testing
	//glDepthFunc(GL_LEQUAL); // Use LEQUAL to ensure correct depth testing for decals SHOULD THIS BE COMMENTED OR UNCOMMENTED
	glDepthFunc(GL_ALWAYS);

	UseShader(*decalShader);

	// Send near and far plane uniforms to decalBlend shader
	// To conver non-linear depth to linear depth for accurate depth testing
	glUniform1f(glGetUniformLocation(decalShader->GetProgramID(), "nearPlane"), camera->GetNearPlane());
	glUniform1f(glGetUniformLocation(decalShader->GetProgramID(), "farPlane"), camera->GetFarPlane());

	GLuint decalTextureLocation = glGetUniformLocation(decalShader->GetProgramID(), "depthTexture");
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex); //was hdrDepthTex. 
	glUniform1i(decalTextureLocation, 1);

	GLuint alphaFadeLocation = glGetUniformLocation(decalShader->GetProgramID(), "alphaFade");
	GLuint decalColorLocation = glGetUniformLocation(decalShader->GetProgramID(), "decalColor");

	GLuint modelMatrixLocation = glGetUniformLocation(decalShader->GetProgramID(), "modelMatrix");
	GLuint viewProjMatrixLocation = glGetUniformLocation(decalShader->GetProgramID(), "viewProjMatrix");

	glUniform1i(glGetUniformLocation(decalShader->GetProgramID(), "screenWidth"), windowSize.x);
	glUniform1i(glGetUniformLocation(decalShader->GetProgramID(), "screenHeight"), windowSize.y);

	// Projection matrix is required because decals require projection from world space onto a surface,
	// which is done by projecting the decal onto the surface using the normal of the surface.
	Matrix4 viewMatrix = camera->BuildViewMatrix();
	Matrix4 projMatrix = camera->BuildProjectionMatrix(hostWindow->GetScreenAspect());
	Matrix4 viewProjMatrix = projMatrix * viewMatrix;
	glUniformMatrix4fv(viewProjMatrixLocation, 1, false, (float*)&viewProjMatrix);

	for (const auto& decal : decalSystem.GetDecals()) {
		// Get the random angle of rotation for the decal
		float randomRotAngle = decal.rotation;

		// Create a rotation matrix to orient the decal based on the normal of the surface it is projected onto
		Matrix4 rotationMatrix = Matrix::RotationAroundNormal(decal.normal, randomRotAngle);

		Matrix4 modelMatrix = Matrix::Translation(Vector3(decal.position)) *
							  rotationMatrix *
							  Matrix::Scale(Vector3(decal.radius, decal.radius, decal.radius));

		glUniform1f(alphaFadeLocation, decal.alphaFade);
		glUniform4fv(decalColorLocation, 1, (float*)&decal.color);
		glUniformMatrix4fv(modelMatrixLocation, 1, false, (float*)&modelMatrix);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ((OGLTexture*)decal.texture.get())->GetObjectID());

		// RenderQuad() renders the decal at the specific hit location to the decalFBO
		RenderQuad();
	}

	// Unbind the FBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0); 

	glDisable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);
}

void GameTechRenderer::RenderPostProcessing() {
	//Vignette post processing:
	    glBindFramebuffer(GL_FRAMEBUFFER, BFBO); //unbind hdrFBO and set BFBO   
	    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	    glDisable(GL_CULL_FACE);
	    glDisable(GL_BLEND);
	    glDisable(GL_DEPTH_TEST);
	    UseShader(*vignetteShader);
	    glUniform1i(glGetUniformLocation(vignetteShader->GetProgramID(), "vignetteOn"), GetVignetteOn());
		glUniform1f(glGetUniformLocation(vignetteShader->GetProgramID(), "vignetteIntensity"), vignetteIntensity);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex); //hdrTex currently holds raw scene
		glUniform1i(glGetUniformLocation(vignetteShader->GetProgramID(), "diffuseTex"), 0);
		glUniform2f(glGetUniformLocation(vignetteShader->GetProgramID(), "windowSize"), windowSize.x, windowSize.y);
		glUniform3fv(glGetUniformLocation(vignetteShader->GetProgramID(), "effectColour"), 1, (float*)&vignetteColour);


		glUniform1f(glGetUniformLocation(vignetteShader->GetProgramID(), "time"), vignettePulse);
		BindMesh(*fullscreenQuad); //simply a quad   
		DrawBoundMesh(); //finished rendering into BTex now, ready to unbind to draw quad straight to screen next:
		//HDR post processing:
		glBindFramebuffer(GL_FRAMEBUFFER, 0); //was 0, now trying rendering back into hdrFBO
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		UseShader(*hdrShader);
		glUniform1i(glGetUniformLocation(hdrShader->GetProgramID(), "hdrOn"), GetHDROn()); //send bool to shader so it knows whether to output scene with or without post processing
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, BTex);//BTex holds vignetted processed scene (whether applied or not). (tonemapping should be done pretty much last)
		glUniform1i(glGetUniformLocation(hdrShader->GetProgramID(), "hdrTex"), 0);
		BindMesh(*fullscreenQuad); //using unitQuad instead of fullscreen quad
		DrawBoundMesh();
	

}

void GameTechRenderer::GenerateScreenTexture(GLuint& into, bool depth) {  
	glGenTextures(1, &into);
	glBindTexture(GL_TEXTURE_2D, into); 

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	GLuint format = depth ? GL_DEPTH_COMPONENT24 : GL_RGBA16F; //using floating point textures to allow HDR rendering 
	GLuint type = depth ? GL_DEPTH_COMPONENT : GL_RGBA; 
	GLuint datatype = depth ? GL_UNSIGNED_BYTE : GL_FLOAT;  

	glTexImage2D(GL_TEXTURE_2D, 0, format, windowSize.x, windowSize.y, 0, type, datatype, NULL); 
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GameTechRenderer::DrawScene() { //the basic rendering for the scene, currently not including shadows. 
	
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);

	glEnable(GL_CULL_FACE);
	glClearColor(1, 1, 1, 0); //doesn't seem to change anything regardless of alpha
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	RenderSkybox();
    RenderCamera();
	// Render Decals to it's own buffer 
	RenderDecals(); 
	
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	// Blend decals onto the scene using a fullscreen quad
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	////glBindFramebuffer(GL_FRAMEBUFFER, 0); //////// Why unbind to 0? COMMENTING THIS OUT AT LEAST ALLOWS THE UI TO RENDER PROPERLY

	UseShader(*decalBlendShader);

	//// Send near and far plane uniforms to decalBlend shader
	//// To conver non-linear depth to linear depth for accurate depth testing
	/*glUniform1f(glGetUniformLocation(decalBlendShader->GetProgramID(), "nearPlane"), camera->GetNearPlane()); //DO THESE TWO LINES STILL DO ANYTHING?
	glUniform1f(glGetUniformLocation(decalBlendShader->GetProgramID(), "farPlane"), camera->GetFarPlane());*/

	GLuint decalTextureLocation = glGetUniformLocation(decalBlendShader->GetProgramID(), "decalTexture");
	glUniform1i(decalTextureLocation, 0);

	//// Bind the decal texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, decalSystem.GetDecalTexture());

	//// Bind the scene texture
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex); //was hdrTex. Can probably use bufferColourTex as long as lighting not required 
	glUniform1i(glGetUniformLocation(decalBlendShader->GetProgramID(), "sceneTexture"), 1);

	////// Bind depth texture
	/*glActiveTexture(GL_TEXTURE2); //DO THESE 3 LINES STILL DO ANYTHING?
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex); //was hdrDepthTex. 
	glUniform1i(glGetUniformLocation(decalBlendShader->GetProgramID(), "depthTexture"), 2);*/

	////glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); //Commenting this out at least lets the skybox appear
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	BindMesh(*unitQuad);
	DrawBoundMesh(); 

    /*glDisable(GL_CULL_FACE); //Todo - text indices are going the wrong way...  //before merge conflicts, this line would have come immediately after renderCamera()
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    NewRenderLines();
    NewRenderTextures();
    NewRenderText();
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    RenderUI();*/
}

void GameTechRenderer::FillBuffers() { //draws unlit scene 
    //bind the framebuffer to store unlit scene
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO); //We will draw the whole scene into bufferFBO for now including the skybox 
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); 
	 
	//draw scene:
	DrawScene(); //perhaps don't need to draw whole scene but just what will be affected by lighting i.e. skybox will not

	//Unbind the framebuffer:
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GameTechRenderer::DrawPointLights() { 
	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);
	UseShader(*pointlightShader);
	glClearColor(0, 0, 0, 1); //set to black so that it doesn't interfere with additive blending for light
	glClear(GL_COLOR_BUFFER_BIT);

	glCullFace(GL_FRONT);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glUniform1i(glGetUniformLocation(pointlightShader->GetProgramID(), "depthTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);

	glUniform1i(glGetUniformLocation(pointlightShader->GetProgramID(), "normTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bufferNormalTex);

	int cameraLocation = glGetUniformLocation(sceneShader->GetProgramID(), "cameraPos");
	Vector3 camPos = camera->GetPosition();
	glUniform3fv(cameraLocation, 1, &camPos.x);
	glUniform2f(glGetUniformLocation(pointlightShader->GetProgramID(), "pixelSize"), 1.0f / windowSize.x, 1.0f / windowSize.y);

	//using the same proj and view matrices from RenderCamera(): 
	Matrix4 viewMatrix = camera->BuildViewMatrix();
	Matrix4 projMatrix = camera->BuildProjectionMatrix(hostWindow->GetScreenAspect());
	Matrix4 invViewProj = Matrix::Inverse(projMatrix * viewMatrix); // trying to take inverse of projview matrix

	glUniformMatrix4fv(glGetUniformLocation(pointlightShader->GetProgramID(), "inverseProjView"), 1, false, (float*)&invViewProj);
	//update shader matrices here (don't need to set model matrices though as this will be taken care of in vertex shader):
	glUniformMatrix4fv(glGetUniformLocation(pointlightShader->GetProgramID(), "viewMatrix"), 1, false, (float*)&viewMatrix);
	glUniformMatrix4fv(glGetUniformLocation(pointlightShader->GetProgramID(), "projMatrix"), 1, false, (float*)&projMatrix);
	BindMesh(*lightSphere);
	for (PointLight* light : lights) {
		glUniform3fv(glGetUniformLocation(pointlightShader->GetProgramID(), "lightPos"), 1, (float*)&light->worldPosition);
		glUniform4fv(glGetUniformLocation(pointlightShader->GetProgramID(), "lightColour"), 1, (float*)&light->colour);
		glUniform1f(glGetUniformLocation(pointlightShader->GetProgramID(), "lightRadius"), light->radius);
		glUniform1f(glGetUniformLocation(pointlightShader->GetProgramID(), "lightIntensity"), light->intensity);
		DrawBoundMesh();
	};

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

}

void GameTechRenderer::CombineBuffers() {//basically final post processing output. Don't need to update matrices as fullscreen quad is not transformed at all
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO); //swapped hdrFBO with bufferFBO to allow decals to work
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); //without this line, output is just black 
	UseShader(*combineShader); 
	glUniform1i(glGetUniformLocation(combineShader->GetProgramID(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTex);

	glUniform1i(glGetUniformLocation(combineShader->GetProgramID(), "diffuseLight"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, lightDiffuseTex);

	glUniform1i(glGetUniformLocation(combineShader->GetProgramID(), "specularLight"), 2); 
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, lightSpecularTex);


	BindMesh(*fullscreenQuad);
	DrawBoundMesh();
	glBindFramebuffer(GL_FRAMEBUFFER, 0); //hdrTex should now hold full deferred lighting rendered scene 

}