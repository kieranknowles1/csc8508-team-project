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
#include <PS5Core/AGCTexture.h>
#include <PS5Core/AGCMesh.h>
#include "GameTechAGCRenderer.h"
using PlatformTexture = NCL::PS5::AGCTexture;
using PlatformMesh = NCL::PS5::AGCMesh;
#endif


namespace NCL::CSC8503 {

ResourceManager::ResourceManager(GameTechRendererInterface* renderer, float threadMult)
	: renderer(renderer)
	, meshes(this)
	, textures(this)
    , materials(this)
	, timeSinceGc(0)
{
	auto pwd = std::filesystem::current_path().string();
	std::cout << "Using working directory: " << pwd << std::endl;

	// Includes hyperthreading
	int hardwareThreads = std::thread::hardware_concurrency();
	int workerThreadCount = std::max(1, int(std::ceil(threadMult * hardwareThreads)));

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
		jobsCv.wait(lock, [&]{ return incompleteJobs == 0; });
	}

	meshes.update();
	textures.update();
}

void ResourceManager::collectGarbage()
{
	timeSinceGc = 0;
	meshes.collectGarbage();
	textures.collectGarbage();
    materials.collectGarbage();
}

void ResourceManager::threadFunc() {
	while (true) {
		auto job = getJob();
		if (job != nullptr) {
			(*job)();
			completeJob();
		} else {
			std::cout << "Worker thread " << std::this_thread::get_id() << " exiting" << std::endl;
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
	return j;
}

template<>
void ResourceMap<std::string, Rendering::Mesh>::load(const std::string& key, Rendering::Mesh* outMesh) {
	Rendering::MshLoader::LoadMesh(key, *outMesh);
}

template<>
void ResourceMap<std::string, Rendering::Texture>::load(const std::string& key, Rendering::Texture* outTexture) {
	outTexture->load(key);
#ifdef __PROSPERO__
	auto renderer = (GameTechAGCRenderer*)owner->getRenderer()->getBase();
	renderer->RegisterTexture(key, (AGCTexture*)outTexture);
#endif // __PROSPERO__

}

template<>
void ResourceMap<std::string, Rendering::Mesh>::upload(Rendering::Mesh* mesh) {
	mesh->UploadToGPU(owner->getRenderer()->getBase());
}

template<>
void ResourceMap<std::string, Rendering::Texture>::upload(Rendering::Texture* tex) {
	tex->upload();
}

template<>
void ResourceMap<std::string, Material>::load(const std::string& key, Material* mat) {
	// We load during construction
}

template<>
void ResourceMap<std::string, Material>::upload(Material* mat) {
	// Nothing to do, all uploads are done by the textures
}


template<>
std::shared_ptr<Rendering::Mesh> ResourceMap<std::string, Rendering::Mesh>::construct(const std::string& key) {
	return std::make_shared<PlatformMesh>();
}

template<>
std::shared_ptr<Rendering::Texture> ResourceMap<std::string, Rendering::Texture>::construct(const std::string& key) {
	return std::make_shared<PlatformTexture>();
}

template<>
std::shared_ptr<Material> ResourceMap<std::string, Material>::construct(const std::string& key) {
	return std::make_shared<Material>(owner, key);
}

}
