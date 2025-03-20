#include "MeshMaterial.h"
#include "Assets.h"
#include <fstream>
#include "../TeamProject/ResourceManager.h"

namespace NCL {

MeshMaterial::MeshMaterial(const std::string& filename) {
	std::ifstream file(Assets::MATERIALDIR + filename);

	std::string dataType;
	file >> dataType;

	if (dataType != "MeshMat") {
		std::cout << __FUNCTION__ << " File " << filename << " is not a MeshMaterial!\n";
		return;
	}
	int version;
	file >> version;

	if (version != 1) {
		std::cout << __FUNCTION__ << " File " << filename << " has incompatible version " << version << "!\n";
		return;
	}

	int matCount;
	int meshCount;
	file >> matCount;
	file >> meshCount;

	materialLayers.resize(matCount);

	for (int i = 0; i < matCount; ++i) {
		std::string name;
		int count;
		file >> name;
		file >> count;

		for (int j = 0; j < count; ++j) {
			std::string entryData;
			file >> entryData;
			size_t split = entryData.find_first_of(':');
			std::string key = entryData.substr(0, split);
			std::string value = entryData.substr(split + 1);

			materialLayers[i].entries.insert(std::make_pair(key, value));
		}
	}

	for (int i = 0; i < meshCount; ++i) {
		int entry;
		file >> entry;
		meshLayers.emplace_back(&materialLayers[entry]);
	}
}

const MeshMaterialEntry* MeshMaterial::GetMaterialForLayer(int i) const {
	if (i < 0 || i >= meshLayers.size()) {
		return nullptr;
	}
	return meshLayers[i];
}

}