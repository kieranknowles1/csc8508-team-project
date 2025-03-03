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
			GameTechRenderer();

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

			~GameTechRenderer();

			//void RenderFrame()	override;

			//Made AddUIElement() public so the pushdown states can use them, is this a problem?

			OGLMesh* LoadMesh(const std::string& name) override; /////WAS Mesh* instead of OGLMesh*
			Texture* LoadTexture(const std::string& name) override;
			void AddUIElement(Vector2 position, Vector2 size, Vector4 color, OGLTexture* texture = nullptr);

		protected:
			void NewRenderLines();
			void NewRenderText();
			void NewRenderTextures();

			void RenderFrame()	override;

			void RenderShadowMap();
			void RenderCamera();
			void RenderSkybox();
			void InitCrosshair(); //InitCrosshair and RenderCrosshair Ameya added for crosshair
			//void AddUIElement(Vector2 position, Vector2 size, Vector4 color, OGLTexture* texture = nullptr);
			void InitUIQuad();
			void RenderUI();

			void LoadSkybox();

			void SetDebugStringBufferSizes(size_t newVertCount);
			void SetDebugLineBufferSizes(size_t newVertCount);

			std::vector<UIElement> uiElements;

			std::unique_ptr<OGLShader> uiShader;
			std::unique_ptr<OGLShader> sceneShader;
			std::unique_ptr<OGLShader> debugShader;
			std::unique_ptr<OGLShader> skyboxShader;
			std::unique_ptr<OGLMesh> skyboxMesh;
			std::unique_ptr<OGLMesh> debugTexMesh;
			std::unique_ptr<OGLMesh> uiQuadMesh;
			GLuint		skyboxTex;

			GLuint crosshairVAO;
			GLuint crosshairVBO;
			GLuint crosshairEBO;

			//shadow mapping things
			std::unique_ptr<OGLShader> shadowShader;
			GLuint		shadowTex;
			GLuint		shadowFBO;
			Matrix4     shadowMatrix;

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
			OGLShader* deferredsceneShader;
			OGLShader* pointlightShader;
			OGLShader* combineShader;
			OGLMesh* lightSphere;
			GLuint bufferFBO;
			GLuint pointLightFBO;
			void GenerateScreenTexture(GLuint& into, bool depth = false);
			GLuint bufferDepthTex;
			GLuint bufferColourTex;
			GLuint bufferNormalTex;
			GLuint lightDiffuseTex;
			GLuint lightSpecularTex;
			void FillBuffers();
			void DrawPointLights();
			void CombineBuffers();
			void DrawScene(); 

			//Post processing additions:
			GLuint hdrTex;
			GLuint hdrFBO;
			GLuint hdrDepthTex;
			OGLMesh* fullscreenQuad; 
			OGLShader* hdrShader;
			GLuint BTex;
			GLuint BFBO;
			OGLShader* vignetteShader;
			GLuint BDepthTex;
			void RenderPostProcessing();

		};
	}
}

