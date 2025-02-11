#pragma once

#include <OpenGLRendering/OGLRenderer.h>

#include "RenderBackend.h"

namespace NCL::CSC8503::Render {
	class OpenGlBackend : public RenderBackend, public OGLRenderer {
    public:
		OpenGlBackend(Window* window);

        void beginFrame() override;
        void swapBuffers() override;

        Rendering::Mesh* loadMesh(std::string_view name) override;
        Rendering::Texture* loadTexture(std::string_view name) override;
        Rendering::Shader* loadShader(std::string_view vertex, std::string_view fragment) override;

        void useShader(Rendering::Shader* shader) override;
        void bindMesh(Rendering::Mesh* mesh) override;
        void drawBoundMesh(int subLayer = 0, int numInstances = 1) override;
        void bindTextureToShader(const Rendering::Texture* t, const std::string& uniform, int texUnit) override;
	};
}
