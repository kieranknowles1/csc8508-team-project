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
		virtual Rendering::Mesh* LoadMesh(const std::string& name) = 0;
		virtual Rendering::Texture*	LoadTexture(const std::string& name) = 0;

		bool GetHDRON() const {
			return hdrON;
		}

		void SetHDRON(bool toggle) {
			hdrON = toggle;
		}
	protected:
		//adding bools to toggle post processing. Must be accessible from the specific renderer
		bool hdrON = true;
	};
}

