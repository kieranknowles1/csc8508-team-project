#include "ResourceManager.h"

#include <filesystem>
#include <iostream>

#include "GameTechRendererInterface.h"
#include "MshLoader.h"
#include "TextureLoader.h"

#ifndef __PROSPERO__
#include "OGLTexture.h"
#include "OGLMesh.h"
using PlatformTexture = NCL::Rendering::OGLTexture;
using PlatformMesh = NCL::Rendering::OGLMesh;
#else
#include "AGCTexture.h"
#include "AGCMesh.h"
using PlatformTexture = NCL::PS5::AGCTexture;
using PlatformMesh = NCL::PS5::AGCMesh;
#endif

namespace NCL::CSC8503 {

ResourceManager::ResourceManager(GameTechRendererInterface* renderer, int workerThreadCount)
	: renderer(renderer)
	, meshes(this)
	, textures(this)
{
	auto pwd = std::filesystem::current_path().string();
	std::cout << "Using working directory: " << pwd << std::endl;

	std::cout << "Spawning " << workerThreadCount << " worker threads" << std::endl;
	for (int i = 0; i < workerThreadCount; i++) {
		workerThreads.emplace_back(&ResourceManager::threadFunc, this);
	}
}

ResourceManager::~ResourceManager()
{
	quitting = true;
	jobsCv.notify_all();
	for (auto& thread : workerThreads) {
		thread.join();
	}
}

void ResourceManager::update(float dt)
{
	timeSinceGc += dt;
	if (timeSinceGc >= gcFrequency) {
		collectGarbage();
	}

	{
		std::unique_lock lock(jobsMtx);

		jobsCv.wait(lock, [&]{ return jobs.empty(); });
	}

	meshes.update();
	textures.update();
}

void ResourceManager::collectGarbage()
{
	timeSinceGc = 0;
	meshes.collectGarbage();
	textures.collectGarbage();
}

void ResourceManager::threadFunc() {
	while (true) {
		auto job = getJob();
		if (job != nullptr) {
			(*job)();
		} else {
			return; // We are quitting
		}
	}
}

std::unique_ptr<Job> ResourceManager::getJob() {
	std::unique_lock lock(jobsMtx);
	jobsCv.wait(lock, [&] { return quitting || !jobs.empty(); });

	if (quitting) {
		return nullptr;
	}

	auto j = std::move(jobs.back());
	jobs.pop_back();
	lock.unlock();
	jobsCv.notify_one();
	return j;
}

template<>
std::shared_ptr<Rendering::Texture> ResourceMap<std::string, Rendering::Texture>::construct() {
	return std::make_shared<PlatformTexture>();
}

template<>
std::shared_ptr<Rendering::Mesh> ResourceMap<std::string, Rendering::Mesh>::construct() {
	return std::make_shared<PlatformMesh>();
}

template<>
void ResourceMap<std::string, Rendering::Mesh>::load(const std::string& key, Rendering::Mesh* outMesh) {
	Rendering::MshLoader::LoadMesh(key, *outMesh);
}

template<>
void ResourceMap<std::string, Rendering::Texture>::load(const std::string& key, Rendering::Texture* outTexture) {
	outTexture->load(key);
}

template<>
void ResourceMap<std::string, Rendering::Mesh>::upload(Rendering::Mesh* mesh) {
	mesh->UploadToGPU();
}

template<>
void ResourceMap<std::string, Rendering::Texture>::upload(Rendering::Texture* tex) {
	tex->upload();
}

}
