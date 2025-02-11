#pragma once

#include <string>
#include <functional>

namespace NCL::Rendering {
    class Mesh;
    class Texture;
    class Shader;
}

namespace NCL::CSC8503::Render {
    class RenderBackend {
    public:
        // Function to be called when the state of the backend changes, useful for configuring uniforms
        using ChangeCallback = std::function<void()>;

        virtual void beginFrame() = 0;
        virtual void swapBuffers() = 0;

        // TODO: Return shared pointers
        virtual Rendering::Mesh* loadMesh(std::string_view name) = 0;
        virtual Rendering::Texture* loadTexture(std::string_view name) = 0;
        virtual Rendering::Shader* loadShader(std::string_view vertex, std::string_view fragment) = 0;

        virtual void drawBoundMesh(int subLayer = 0, int numInstances = 1) = 0;
        virtual void bindTextureToShader(const Rendering::Texture* t, const std::string& uniform, int texUnit) = 0;

        inline void setActiveShader(Rendering::Shader* shader, ChangeCallback cb = ChangeCallback()) {
            if (shader != activeShader) {
                activeShader = shader;
                useShader(activeShader);
                if (cb) cb();
            }
        }

        inline void setActiveMesh(Rendering::Mesh* mesh, ChangeCallback cb = ChangeCallback()) {
            if (mesh != activeMesh) {
                activeMesh = mesh;
                bindMesh(mesh);
                if (cb) cb();
            }
        }

    protected:
        // TODO: Provide a DrawMeshList function, maybe make these private
        virtual void useShader(Rendering::Shader* shader) = 0;
        virtual void bindMesh(Rendering::Mesh* mesh) = 0;

        Rendering::Shader* activeShader = nullptr;
        Rendering::Mesh* activeMesh = nullptr;
    };
}
