#include "ResourceManager.h"

#include <filesystem>
#include <iostream>

#include "GameTechRendererInterface.h"
#include "MshLoader.h"
#include "TextureLoader.h"

#ifdef __PROSPERO__
#include "GameTechAGCRenderer.h"
#endif // __PROSPERO__


namespace NCL::CSC8503 {

ResourceManager::ResourceManager(GameTechRendererInterface* renderer, float threadMult)
	: renderer(renderer)
	, meshes(this)
	, textures(this)
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
void ResourceMap<std::string, PlatformMesh>::load(const std::string& key, PlatformMesh* outMesh) {
	Rendering::MshLoader::LoadMesh(key, *outMesh);
}

template<>
void ResourceMap<std::string, PlatformTexture>::load(const std::string& key, PlatformTexture* outTexture) {
	outTexture->load(key);
#ifdef __PROSPERO__
	auto renderer = (GameTechAGCRenderer*)owner->getRenderer()->getBase();
	renderer->RegisterTexture(key, outTexture);
#endif // __PROSPERO__

}

template<>
void ResourceMap<std::string, PlatformMesh>::upload(PlatformMesh* mesh) {
	mesh->UploadToGPU(owner->getRenderer()->getBase());
}

template<>
void ResourceMap<std::string, PlatformTexture>::upload(PlatformTexture* tex) {
	tex->upload();
}

}
