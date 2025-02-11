#pragma once

#include <memory>

#include "RenderBackend.h"

namespace NCL::CSC8503 {
    class GameWorld;
}

namespace NCL::CSC8503::Render {
    class GameRenderer {
    public:
        GameRenderer(std::unique_ptr<RenderBackend> backend);
        ~GameRenderer();

        RenderBackend* getBackend() { return backend; }

        void drawWorld(const GameWorld* world);
    private:
        std::unique_ptr<RenderBackend> backend;

        void NewRenderLines();
        void NewRenderText();
        void NewRenderTextures();

        void RenderFrame();

        OGLShader*		defaultShader;

        GameWorld&	gameWorld;

        void BuildObjectList();
        void SortObjectList();
        void RenderShadowMap();
        void RenderCamera();
        void RenderSkybox();

        void LoadSkybox();

        void SetDebugStringBufferSizes(size_t newVertCount);
        void SetDebugLineBufferSizes(size_t newVertCount);

        std::vector<const RenderObject*> activeObjects;

        OGLShader*  debugShader;
        OGLShader*  skyboxShader;
        OGLMesh*	skyboxMesh;
        OGLMesh*	debugTexMesh;
        GLuint		skyboxTex;

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
