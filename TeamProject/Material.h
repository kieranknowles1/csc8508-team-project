/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once

namespace NCL {
	namespace CSC8503 {
		class ResourceManager;
	};

	namespace Rendering {
		class Texture;
	}

	class Material {
	public:
		struct Layer {
			std::shared_ptr<Rendering::Texture> diffuse;
			std::shared_ptr<Rendering::Texture> normal;
		};

		// Load from a MeshMaterial file
		// The following keys are supported:
		//  - Diffuse: The path to the diffuse texture
		//  - Bump: The path to the normal map
		//  - InvertY: If true, the texture will be flipped vertically, default is false
		Material(CSC8503::ResourceManager* rm, const std::string& filename);		

		int GetLayerCount() const { return layers.size(); }
		const Layer* GetLayer(int i) {
			if (i >= layers.size()) {
				return nullptr;
			}
			return &layers[i];
		}
	private:
		std::vector<Layer> layers;
	};


}