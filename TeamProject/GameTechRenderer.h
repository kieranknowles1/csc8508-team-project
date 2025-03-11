#pragma once

#ifdef __PROSPERO__
#error GameTechRenderer.h cannot be included on PS5
#endif

#include <memory>

#include "OGLRenderer.h"
#include "OGLShader.h"
#include "OGLTexture.h"
#include "OGLMesh.h"

#include "GameTechRendererInterface.h"

namespace NCL {
	namespace CSC8503 {
		class RenderObject;

		class GameTechRenderer
			: public OGLRenderer
			, public GameTechRendererInterface {
		public:
			void drawFrame(float dt) override {
				Update(dt);
				Render();
			}

			GameTechRenderer(Window* window);
			~GameTechRenderer() override;

			OGLMesh* LoadMesh(const std::string& name) override; /////WAS Mesh* instead of OGLMesh*
			Texture* LoadTexture(const std::string& name) override;

		protected:
			void NewRenderLines();
			void NewRenderText();
			void NewRenderTextures();

			void RenderFrame()	override;

			void RenderShadowMap();
			void RenderCamera();
			void RenderSkybox();

			void RenderDecals();
			void RenderQuad();
			void RenderUI();

			void LoadSkybox();

			void SetDebugStringBufferSizes(size_t newVertCount);
			void SetDebugLineBufferSizes(size_t newVertCount);

			std::unique_ptr<OGLShader> uiShader;
			std::unique_ptr<OGLShader> sceneShader;
			std::unique_ptr<OGLShader> debugShader;
			std::unique_ptr<OGLShader> skyboxShader;
			std::unique_ptr<OGLMesh> skyboxMesh;

			// 1.0f size quad, for HDR
			std::unique_ptr<OGLMesh> unitQuad;
			// 0.5f size quad, for sprites
			std::unique_ptr<OGLMesh> halfUnitQuad;
			GLuint		skyboxTex;

			GLuint crosshairVAO;
			GLuint crosshairVBO;
			GLuint crosshairEBO;

			//shadow mapping things
			std::unique_ptr<OGLShader> shadowShader;
			GLuint		shadowTex;
			GLuint		shadowFBO;
			Matrix4     shadowMatrix;

			// Decal stuff
			std::unique_ptr<OGLShader> decalShader;
			std::unique_ptr<OGLShader> decalBlendShader;
			GLuint decalQuadVAO = 0;
			GLuint decalQuadVBO = 0;

			Vector4		lightColour;
			float		lightRadius;
			Vector3		lightPosition;

			//Debug data storage things
			std::vector<Vector3> debugLineData;

			std::vector<Vector3> debugTextPos;
			std::vector<Vector4> debugTextColours;
			std::vector<Vector2> debugTextUVs;

			GLuint lineVAO;
			GLuint lineVertVBO;
			size_t lineCount = 0;

			GLuint textVAO;
			GLuint textVertVBO;
			GLuint textColourVBO;
			GLuint textTexVBO;
			size_t textCount = 0;

			//Deferred rendering additions:
			std::unique_ptr<OGLShader> deferredsceneShader;
			std::unique_ptr<OGLShader> pointlightShader;
			std::unique_ptr<OGLShader> combineShader;

			std::unique_ptr<OGLMesh> lightSphere;
			GLuint bufferFBO;

			// For drawing multiple point lights.
			GLuint pointLightFBO;		// For drawing point light specular and diffuse to textures.
			GLuint lightDiffuseTex;		// Used by pointLightFBO for the diffuse texture.
			GLuint lightSpecularTex;	// Used by pointLightFBO for the specular texture.


			void GenerateScreenTexture(GLuint& into, bool depth = false); //added
			GLuint bufferDepthTex;
			GLuint bufferColourTex;
			GLuint bufferNormalTex;
			void FillBuffers();
			void DrawPointLights();
			void CombineBuffers();
			void DrawScene();

			//Post processing additions:
			GLuint hdrTex;
			GLuint hdrFBO;
			GLuint hdrDepthTex;
			std::unique_ptr<OGLMesh> fullscreenQuad;
			std::unique_ptr<OGLShader> hdrShader;
			GLuint BTex;
			GLuint BFBO;
			std::unique_ptr<OGLShader> vignetteShader;
			GLuint BDepthTex;
			void RenderPostProcessing();

		};
	}
}

