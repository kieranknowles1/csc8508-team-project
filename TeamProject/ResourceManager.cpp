#include "ResourceManager.h"

#include <filesystem>
#include <iostream>

#include "GameTechRendererInterface.h"

namespace NCL::CSC8503 {

ResourceManager::ResourceManager(GameTechRendererInterface* renderer)
	: renderer(renderer)
	, meshes(this)
	, textures(this)
{
	auto pwd = std::filesystem::current_path().string();
	std::cout << "Using working directory: " << pwd << std::endl;
}

template<>
std::shared_ptr<Rendering::Mesh> ResourceMap<std::string, Rendering::Mesh>::load(const std::string& key) {
	auto mesh = owner->getRenderer()->LoadMesh(key);
	return std::shared_ptr<Rendering::Mesh>(mesh);
}

template<>
std::shared_ptr<Rendering::Texture> ResourceMap<std::string, Rendering::Texture>::load(const std::string& key) {
	auto texture = owner->getRenderer()->LoadTexture(key);
	return std::shared_ptr<Rendering::Texture>(texture);
}

}
