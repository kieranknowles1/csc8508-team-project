#pragma once

namespace NCL::Rendering {
	class Mesh;
	class Texture;
	class Shader;
}

namespace NCL::CSC8503 {
	class GameTechRendererInterface
	{
	public:
		virtual Rendering::Mesh*		LoadMesh(const std::string& name)		= 0;
		virtual Rendering::Texture*	LoadTexture(const std::string& name)	= 0;
		virtual Rendering::Shader*		LoadShader(const std::string& vertex, const std::string& fragment) = 0;
	};
}

