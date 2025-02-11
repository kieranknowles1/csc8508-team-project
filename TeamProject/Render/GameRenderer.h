#pragma once

#include <memory>

#include "RenderBackend.h"

#include <CSC8503CoreClasses/RenderObject.h>

// TODO: Move these to the OpenGL backend
#include <OpenGLRendering/OGLShader.h>
#include <OpenGLRendering/OGLMesh.h>

namespace NCL {
    class Camera;
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

        void drawWorld(GameWorld* world);
    private:
        Window* window;

        RendererOptions options;
        std::unique_ptr<RenderBackend> backend;

        // Data that doesn't persist between frames, but the buffers
        // for which are kept to save on allocation
        using ObjectList = std::vector<const RenderObject*>;
        struct ScratchSpace {
            void clear() {
                renderObjects.clear();
            }
            ObjectList renderObjects;
        };
        ScratchSpace scratch;

        void NewRenderLines(Camera& camera);
        void NewRenderText();
        void NewRenderTextures();

        Rendering::Shader* defaultShader;

        void buildObjectList(const GameWorld* world, ObjectList& out) const;
        void renderShadowMap(const ObjectList& objects);
        void renderCamera(Camera& camera, const ObjectList& objects);
        void renderSkybox(Camera& camera);

        void LoadSkybox();

        void SetDebugStringBufferSizes(size_t newVertCount);
        void SetDebugLineBufferSizes(size_t newVertCount);

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
