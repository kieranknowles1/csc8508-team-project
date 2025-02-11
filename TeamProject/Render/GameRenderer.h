#pragma once

#include <memory>

#include "RenderBackend.h"

#include <CSC8503CoreClasses/RenderObject.h>

// TODO: Move these to the OpenGL backend
#include <OpenGLRendering/OGLShader.h>
#include <OpenGLRendering/OGLMesh.h>

namespace NCL {
    class Window;
}

namespace NCL::CSC8503 {
    class GameWorld;
}

namespace NCL::CSC8503::Render {
    struct RendererOptions {
        unsigned int shadowSize = 2048;
    };

    class GameRenderer {
    public:
        GameRenderer(Window* window, std::unique_ptr<RenderBackend> backend, const RendererOptions& options = RendererOptions());
        ~GameRenderer();

        RenderBackend* getBackend() { return backend.get(); }

        void drawWorld(const GameWorld* world);
    private:
        Window* window;
        GameWorld* gameWorld; // TODO: Remove

        RendererOptions options;
        std::unique_ptr<RenderBackend> backend;

        void NewRenderLines();
        void NewRenderText();
        void NewRenderTextures();

        void RenderFrame();

        Rendering::Shader* defaultShader;

        void BuildObjectList();
        void SortObjectList();
        void RenderShadowMap();
        void RenderCamera();
        void RenderSkybox();

        void LoadSkybox();

        void SetDebugStringBufferSizes(size_t newVertCount);
        void SetDebugLineBufferSizes(size_t newVertCount);

        std::vector<const RenderObject*> activeObjects;

        Shader*  debugShader;
        Shader*  skyboxShader;
        Mesh*	skyboxMesh;
        Mesh*	debugTexMesh;
        GLuint		skyboxTex;

        //shadow mapping things
        Shader*	shadowShader;
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
