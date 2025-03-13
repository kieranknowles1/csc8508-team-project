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
    }

	namespace Rendering {
		class Texture;
	}
	class MeshMaterialEntry {
		friend class MeshMaterial;
	public:
		bool GetEntry(const std::string& name, const std::string** output) const {
			auto i = entries.find(name);
			if (i == entries.end()) {
				return false;
			}
			*output = &i->second.first;
			return true;
		}
		std::shared_ptr<Rendering::Texture> GetEntry(const std::string& name) const {
			auto i = entries.find(name);
			if (i == entries.end()) {
				return nullptr;
			}
			return i->second.second;
		}
		void LoadTextures(CSC8503::ResourceManager* resourceManager);

	protected:
		std::map<std::string, std::pair<std::string, std::shared_ptr<Rendering::Texture>>> entries;
	};

	class MeshMaterial	{
    public:
		MeshMaterial(CSC8503::ResourceManager* resourceManager, const std::string& filename);
		~MeshMaterial() {}
		const MeshMaterialEntry* GetMaterialForLayer(int i) const;

		void LoadTextures(CSC8503::ResourceManager* resourceManager);

	protected:
		std::vector<MeshMaterialEntry>	materialLayers;
		std::vector<MeshMaterialEntry*> meshLayers;
	};

}