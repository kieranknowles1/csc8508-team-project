#include "Renderer.h"

#include <stdexcept>

Renderer::Renderer(NCL::Window &window)
    : NCL::Rendering::OGLRenderer(window)
{
    if (!HasInitialised()) {
        throw std::runtime_error("Renderer failed to initialise!");
    }

    shader = this->LoadShader("default.vert", "default.frag");
    cube = this->LoadMesh("Cube.msh");
}

void Renderer::RenderFrame()
{
    UseShader(*shader);
    BindMesh(*cube);
    DrawBoundMesh();
}
