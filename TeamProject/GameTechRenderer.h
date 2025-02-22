#pragma once

#include <memory>

#include "OGLRenderer.h"
#include "OGLShader.h"
#include "OGLTexture.h"
#include "OGLMesh.h"

#include "GameWorld.h"
#include "GameTechRendererInterface.h"

namespace NCL {
	namespace CSC8503 {
		class RenderObject;

		class GameTechRenderer
			: public OGLRenderer
			, public GameTechRendererInterface {
		public:
			GameTechRenderer(GameWorld* world);
			~GameTechRenderer();

			Mesh* LoadMesh(const std::string& name) override;
			Texture* LoadTexture(const std::string& name) override;

			//toggle post processing:
			bool GetHDRON() const{
				return hdrON; 
			}

			void SetHDRON(bool ison) {
				hdrON = ison;
			}

		protected:
			void NewRenderLines();
			void NewRenderText();
			void NewRenderTextures();

			void RenderFrame()	override;

			GameWorld*	gameWorld;

			void BuildObjectList();
			void SortObjectList();
			void RenderShadowMap();
			void RenderCamera(); 
			void RenderSkybox();
			void InitCrosshair(); //InitCrosshair and RenderCrosshair Ameya added for crosshair
			void RenderCrosshair();

			void LoadSkybox();

			void SetDebugStringBufferSizes(size_t newVertCount);
			void SetDebugLineBufferSizes(size_t newVertCount);

			std::vector<const RenderObject*> activeObjects;

			std::unique_ptr<OGLShader> sceneShader;
			std::unique_ptr<OGLShader> debugShader;
			std::unique_ptr<OGLShader> skyboxShader;
			std::unique_ptr<OGLMesh> skyboxMesh;
			std::unique_ptr<OGLMesh> debugTexMesh;
			GLuint		skyboxTex;

			GLuint crosshairVAO;
			GLuint crosshairVBO;
			GLuint crosshairEBO;
			std::unique_ptr<OGLShader> crosshairShader; //This line Ameya added for crosshair

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

			//Post processing additions:
			GLuint hdrTex;
			GLuint hdrFBO;
			GLuint hdrDepthTex;
			OGLMesh* hdrQuad;
			OGLShader* hdrShader;
			bool hdrON = true;
		};
	}
}

