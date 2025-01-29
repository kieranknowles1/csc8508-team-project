#pragma once

#include <OpenGLRendering/OGLRenderer.h>

using RendererBase = NCL::Rendering::OGLRenderer;
using Mesh = NCL::OGLMesh;
using Shader = NCL::OGLShader;

class Renderer : public RendererBase {
public:
    Renderer(NCL::Window& window);

    void RenderFrame() override;

private:
    Mesh* cube;
    Shader* shader;
};
