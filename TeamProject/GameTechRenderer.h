#pragma once
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
			Shader* LoadShader(const std::string& vertex, const std::string& fragment) override;

		protected:
			void NewRenderLines();
			void NewRenderText();
			void NewRenderTextures();

			void RenderFrame()	override;

			OGLShader*		defaultShader;

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

			OGLShader*  debugShader;
			OGLShader*  skyboxShader;
			OGLMesh*	skyboxMesh;
			OGLMesh*	debugTexMesh;
			GLuint		skyboxTex;

			GLuint crosshairVAO;
			GLuint crosshairVBO;
			GLuint crosshairEBO;
			OGLShader* crosshairShader; //This line Ameya added for crosshair

			//shadow mapping things
			OGLShader*	shadowShader;
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
		};
	}
}

