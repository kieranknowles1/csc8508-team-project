/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once

#include <string>
#include <optional>

namespace NCL {
	class MeshMaterialEntry {
		friend class MeshMaterial;
	public:
		bool GetEntry(const std::string& name, const std::string** output) const {
			auto i = entries.find(name);
			if (i == entries.end()) {
				return false;
			}
			*output = &i->second;
			return true;
		}

	protected:
		std::map<std::string, std::string> entries;
	};


	class MeshMaterial	{
    public:
		MeshMaterial( const std::string& filename);
		~MeshMaterial() {}
		const MeshMaterialEntry* GetMaterialForLayer(int i) const;

        int GetLayerCount() const {
            return meshLayers.size();
        }

	protected:
		std::vector<MeshMaterialEntry>	materialLayers;
		std::vector<MeshMaterialEntry*> meshLayers;
	};

}