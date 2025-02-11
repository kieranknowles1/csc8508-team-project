#pragma once

#include <string>

namespace NCL::Rendering {
    class Mesh;
    class Texture;
    class Shader;
}

namespace NCL::CSC8503::Render {
    class RenderBackend {
    public:
        // TODO: Return shared pointers
        virtual Rendering::Mesh* loadMesh(std::string_view name) = 0;
        virtual Rendering::Texture* loadTexture(std::string_view name) = 0;
        virtual Rendering::Shader* loadShader(std::string_view vertex, std::string_view fragment) = 0;
    };
}
