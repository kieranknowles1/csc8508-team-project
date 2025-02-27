#include "GameTechRenderer.h"
#include "GameObject.h"
#include "RenderObject.h"
#include "Camera.h"
#include "TextureLoader.h"
#include "MshLoader.h"

#include "Debug.h"

#include <NCLCoreClasses/stb/stb_image.h>

using namespace NCL;
using namespace Rendering;
using namespace CSC8503;

#define SHADOWSIZE 4096

Matrix4 biasMatrix = Matrix::Translation(Vector3(0.5f, 0.5f, 0.5f)) * Matrix::Scale(Vector3(0.5f, 0.5f, 0.5f));

struct UIElement {
	Vector2 position;
	Vector2 size;
	Vector4 color;
	//GLuint* texture;
	OGLTexture* texture;

	Vector2 GetPosition() { return position; }
	Vector2 GetSize() { return size; }
	Vector4 GetColor() { return color; }
	OGLTexture* GetTexture() { return texture; }
};

std::vector<UIElement> uiElements;

GameTechRenderer::GameTechRenderer(GameWorld* world) : OGLRenderer(*Window::GetWindow()), gameWorld(world)	{
	glEnable(GL_DEPTH_TEST);

	debugShader = std::make_unique<OGLShader>("Debug.vert", "Debug.frag");
	shadowShader = std::make_unique<OGLShader>("shadow.vert", "shadow.frag");
	sceneShader = std::make_unique<OGLShader>("scene.vert", "scene.frag");
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

	//Debug quad for drawing tex
	debugTexMesh = std::make_unique<OGLMesh>();
	debugTexMesh->SetVertexPositions({ Vector3(-1, 1,0), Vector3(-1,-1,0) , Vector3(1,-1,0) , Vector3(1,1,0) });
	debugTexMesh->SetVertexTextureCoords({ Vector2(0, 1), Vector2(0,0) , Vector2(1,0) , Vector2(1,1) });
	debugTexMesh->SetVertexIndices({ 0,1,2,2,3,0 });
	debugTexMesh->UploadToGPU();

	InitUIQuad();


	SetDebugStringBufferSizes(10000);
	SetDebugLineBufferSizes(1000);

	InitCrosshair(); //This line Ameya added for crosshair

	//Post processing additions: 
	hdrShader = new OGLShader("texturevert.glsl", "hdrfrag.glsl");
	
	hdrQuad = new OGLMesh();
	hdrQuad->SetVertexPositions({ Vector3(-1, 1,0), Vector3(-1,-1,0) , Vector3(1,-1,0) , Vector3(1,1,0) }); 
	hdrQuad->SetVertexTextureCoords({ Vector2(0, 1), Vector2(0,0) , Vector2(1,0) , Vector2(1,1) }); 
	hdrQuad->SetVertexIndices({ 0,1,2,2,3,0 }); 
	hdrQuad->UploadToGPU(); 

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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowSize.x, windowSize.y, 0, GL_RGBA, GL_FLOAT, NULL);//making this one floating point as well. Will assume that values maintained even if final output is clamped

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
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !BTex || !BDepthTex); {
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

GameTechRenderer::~GameTechRenderer()	{
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);

	glDeleteTextures(1, &hdrTex);
	glDeleteFramebuffers(1, &hdrFBO);
	glDeleteTextures(1, &BTex);
	glDeleteFramebuffers(1, &BFBO);
	delete hdrQuad;
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
	glEnable(GL_CULL_FACE);
	glClearColor(1, 1, 1, 1);
	BuildObjectList();
	SortObjectList();
	RenderShadowMap();
	//Set up to render into framebuffer
	if (hdrOn || vignetteOn) {
		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO); 
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
	//
	RenderSkybox();
	RenderCamera();
	glDisable(GL_CULL_FACE); //Todo - text indices are going the wrong way...
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	NewRenderLines();
	NewRenderTextures(); 
	NewRenderText();
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	RenderUI();
	RenderPostProcessing();  
}

void GameTechRenderer::BuildObjectList() {
	activeObjects.clear();

	gameWorld->OperateOnContents(
		[&](GameObject* o) {
			if (o->IsActive()) {
				const RenderObject* g = o->GetRenderObject();
				if (g) {
					activeObjects.emplace_back(g);
				}
			}
		}
	);
}

void GameTechRenderer::SortObjectList() {

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

	for (const auto&i : activeObjects) {
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
	glBindFramebuffer(GL_FRAMEBUFFER, 0);// 0

	glCullFace(GL_BACK);
}

void GameTechRenderer::RenderSkybox() {
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	Matrix4 viewMatrix = gameWorld->GetMainCamera().BuildViewMatrix();
	Matrix4 projMatrix = gameWorld->GetMainCamera().BuildProjectionMatrix(hostWindow.GetScreenAspect());

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
	Matrix4 viewMatrix = gameWorld->GetMainCamera().BuildViewMatrix();
	Matrix4 projMatrix = gameWorld->GetMainCamera().BuildProjectionMatrix(hostWindow.GetScreenAspect());

	UseShader(*sceneShader);
	int projLocation	= glGetUniformLocation(sceneShader->GetProgramID(), "projMatrix");
	int viewLocation	= glGetUniformLocation(sceneShader->GetProgramID(), "viewMatrix");
	int modelLocation	= glGetUniformLocation(sceneShader->GetProgramID(), "modelMatrix");
	int colourLocation  = glGetUniformLocation(sceneShader->GetProgramID(), "objectColour");
	int hasVColLocation = glGetUniformLocation(sceneShader->GetProgramID(), "hasVertexColours");
	int hasTexLocation  = glGetUniformLocation(sceneShader->GetProgramID(), "hasTexture");
	int shadowLocation  = glGetUniformLocation(sceneShader->GetProgramID(), "shadowMatrix");
	int hasFlatLocation = glGetUniformLocation(sceneShader->GetProgramID(), "isFlat");
	int hasNormalLocation = glGetUniformLocation(sceneShader->GetProgramID(), "hasNormalMap");
	int texRepeatingLocation = glGetUniformLocation(sceneShader->GetProgramID(), "texRepeating");
	int texScaleLocation = glGetUniformLocation(sceneShader->GetProgramID(), "texScale");

	int lightPosLocation	= glGetUniformLocation(sceneShader->GetProgramID(), "lightPos");
	int lightColourLocation = glGetUniformLocation(sceneShader->GetProgramID(), "lightColour");
	int lightRadiusLocation = glGetUniformLocation(sceneShader->GetProgramID(), "lightRadius");

	int cameraLocation = glGetUniformLocation(sceneShader->GetProgramID(), "cameraPos");

	Vector3 camPos = gameWorld->GetMainCamera().GetPosition();
	glUniform3fv(cameraLocation, 1, &camPos.x);

	glUniformMatrix4fv(projLocation, 1, false, (float*)&projMatrix);
	glUniformMatrix4fv(viewLocation, 1, false, (float*)&viewMatrix);

	glUniform3fv(lightPosLocation, 1, (float*)&lightPosition);
	glUniform4fv(lightColourLocation, 1, (float*)&lightColour);
	glUniform1f(lightRadiusLocation, lightRadius);

	int shadowTexLocation = glGetUniformLocation(sceneShader->GetProgramID(), "shadowTex");
	glUniform1i(shadowTexLocation, 1);

	//TODO - PUT IN FUNCTION
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	for (const auto&i : activeObjects) {
		if ((*i).GetDefaultTexture()) { 
			BindTextureToShader(*(OGLTexture*)(*i).GetDefaultTexture(), "mainTex", 0);
			//figure out scale of object:
			Vector3 scale = i->getParent()->getRenderScale() * i->GetTexScaleMultiplier();
			glUniform3fv(texScaleLocation, 1, scale.array);
			
		}

		//normal map capabilities added:
		if ((*i).GetNormalMap()) {
			BindTextureToShader(*(OGLTexture*)(*i).GetNormalMap(), "normalTex", 2); //need a shader that utilises normal maps, has a uniform sampler2D called "normalTex" in texture unit 2
		}

		//Matrix4 modelMatrix = (*i).GetTransform()->GetMatrix();
		Matrix4 modelMatrix;
		i->getParent()->GetTransform().getOpenGLMatrix((btScalar*)&modelMatrix);
		modelMatrix = modelMatrix * Matrix::Scale(i->getParent()->getRenderScale());
		glUniformMatrix4fv(modelLocation, 1, false, (float*)&modelMatrix);

		Matrix4 fullShadowMat = shadowMatrix * modelMatrix;
		glUniformMatrix4fv(shadowLocation, 1, false, (float*)&fullShadowMat);

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

Mesh* GameTechRenderer::LoadMesh(const std::string& name) {
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

	Matrix4 viewMatrix = gameWorld->GetMainCamera().BuildViewMatrix();
	Matrix4 projMatrix = gameWorld->GetMainCamera().BuildProjectionMatrix(hostWindow.GetScreenAspect());

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

	BindMesh(*debugTexMesh);

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

void GameTechRenderer::AddUIElement(Vector2 position, Vector2 size, Vector4 color, OGLTexture* texture) {
	uiElements.push_back({ position, size, color, texture });
}

void GameTechRenderer::InitCrosshair() {
	Vector2 screenCenter = Vector2(0.5f, 0.5f);
	Vector4 crosshairColor = Vector4(1, 1, 1, 1); // White crosshair

	float lineLength = 0.02f; // Length of the crosshair lines
	float lineThickness = 0.0025f; // Thickness of each line
	float horizontalLineThickness = 0.0035f;
	float horizontalLineLength = 0.015f;
	float gapSize = 0.0005f; // Gap between the lines


	// Left line
	AddUIElement(Vector2(screenCenter.x - gapSize - lineLength, screenCenter.y),
		Vector2(horizontalLineLength, horizontalLineThickness), crosshairColor);

	// Right line
	AddUIElement(Vector2(screenCenter.x + gapSize + lineLength, screenCenter.y),
		Vector2(horizontalLineLength, horizontalLineThickness), crosshairColor);

	// Top line
	AddUIElement(Vector2(screenCenter.x, screenCenter.y + gapSize + lineLength),
		Vector2(lineThickness, lineLength), crosshairColor);

	// Bottom line
	AddUIElement(Vector2(screenCenter.x, screenCenter.y - gapSize - lineLength),
		Vector2(lineThickness, lineLength), crosshairColor);
}

void GameTechRenderer::InitUIQuad() {
	uiQuadMesh = std::make_unique<OGLMesh>();

	// Define a full-screen quad in NDC (-1 to 1)
	std::vector<Vector3> positions = {
		Vector3(-0.5f,  0.5f, 0.0f), // Top Left
		Vector3(-0.5f, -0.5f, 0.0f), // Bottom Left
		Vector3(0.5f, -0.5f, 0.0f), // Bottom Right
		Vector3(0.5f,  0.5f, 0.0f)  // Top Right
	};

	std::vector<Vector2> texCoords = {
		Vector2(0.0f, 1.0f), // Top Left
		Vector2(0.0f, 0.0f), // Bottom Left
		Vector2(1.0f, 0.0f), // Bottom Right
		Vector2(1.0f, 1.0f)  // Top Right
	};

	std::vector<unsigned int> indices = {
		0, 1, 2,  // First Triangle
		2, 3, 0   // Second Triangle
	};

	// Assign to mesh
	uiQuadMesh->SetVertexPositions(positions);
	uiQuadMesh->SetVertexTextureCoords(texCoords);
	uiQuadMesh->SetVertexIndices(indices);

	uiQuadMesh->SetPrimitiveType(GeometryPrimitive::Triangles);
	uiQuadMesh->UploadToGPU();
}

void GameTechRenderer::RenderUI() {
	UseShader(*uiShader);

	// Get uniform locations once and store them
	int positionLocation = glGetUniformLocation(uiShader->GetProgramID(), "position");
	int sizeLocation = glGetUniformLocation(uiShader->GetProgramID(), "size");
	int colorLocation = glGetUniformLocation(uiShader->GetProgramID(), "color");
	int hasTextureLocation = glGetUniformLocation(uiShader->GetProgramID(), "hasTexture");
	int textureLocation = glGetUniformLocation(uiShader->GetProgramID(), "mainTex");

	glDisable(GL_DEPTH_TEST); // UI should always be on top
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (const auto& uiElement : uiElements) {
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
			glBindTexture(GL_TEXTURE_2D, ((OGLTexture*)uiElement.texture)->GetObjectID());
			glUniform1i(textureLocation, 0);
		}
		else {
			glUniform1i(hasTextureLocation, 0);
		}

		// Render UI quad
		BindMesh(*uiQuadMesh);
		DrawBoundMesh();
	}

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void GameTechRenderer::RenderPostProcessing() { 
	if (vignetteOn) {
		GLuint buff = (hdrOn ? BFBO : 0);
		glBindFramebuffer(GL_FRAMEBUFFER, buff); //unbind hdrFBO and set BFBO   
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); //BFBO has neither depth nor stencil attachment
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		UseShader(*vignetteShader); 
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hdrTex); //hdrTex currently holds raw scene
		glUniform1i(glGetUniformLocation(vignetteShader->GetProgramID(), "diffuseTex"), 0); 
		glUniform1f(glGetUniformLocation(vignetteShader->GetProgramID(), "windowSizex"), windowSize.x);
		glUniform1f(glGetUniformLocation(vignetteShader->GetProgramID(), "windowSizey"), windowSize.y);
		Vector3 VignetteColour = Vector3(0.0, 0.0, 0.0); //different colours appear more intense at a given intensity (green)
		float vignetteIntensity = 2.0f; //Recommendation: vary Intensity between 0.8 and 3
		glUniform3fv(glGetUniformLocation(vignetteShader->GetProgramID(), "effectColour"), 1, (float*)&VignetteColour);
		glUniform1f(glGetUniformLocation(vignetteShader->GetProgramID(), "intensity"), vignetteIntensity);
		glUniform1f(glGetUniformLocation(vignetteShader->GetProgramID(), "time"), vignettePulse);  
		BindMesh(*hdrQuad); //simply a quad
		DrawBoundMesh(); //finished rendering into BTex now, ready to unbind to draw quad straight to screen next:
	}
	if (hdrOn) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLuint tex = (vignetteOn ? BTex : hdrTex);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		UseShader(*hdrShader);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);//BTex would hold vignetted scene. (tonemapping should be done pretty much last)
		glUniform1i(glGetUniformLocation(hdrShader->GetProgramID(), "hdrTex"), 0);
		BindMesh(*hdrQuad);
		DrawBoundMesh();
	}
	
}
