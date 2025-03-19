#pragma once

#include <map>
#include <string>
#include <memory>
#include <iostream>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>

#include "GameTechRendererInterface.h"

// Forward declarations won't work here, as we need the full declaration to generate templates
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"
#include "Material.h"

#ifndef __PROSPERO__
#include "OGLTexture.h"
#include "OGLMesh.h"
using PlatformTexture = NCL::Rendering::OGLTexture;
using PlatformMesh = NCL::Rendering::OGLMesh;
#else
#include <PS5Core/AGCTexture.h>
#include <PS5Core/AGCMesh.h>
using PlatformTexture = NCL::PS5::AGCTexture;
using PlatformMesh = NCL::PS5::AGCMesh;
#endif

namespace NCL::CSC8503 {
    class GameTechRenderer;

    class ResourceManager;

    struct Job {
        virtual ~Job() = default;
        virtual void operator()() = 0;
    };

    // A map of resources, where the key is used to load the resource
    // `K` is the type of the key, which must be comparable
    // `V` is the type of the resource, which must have a constructor that takes a reference to a `K`
    // The resource is constructed on the first call to `get` for a given key, and destroyed when the last
    // reference other than the one in the map is destroyed
    template <typename K, typename V>
    class ResourceMap
    {
    public:
        struct LoadResourceJob : public Job {
            LoadResourceJob(ResourceMap<K, V>* outMap, K key, std::shared_ptr<V> resource)
                : outMap(outMap)
                , key(key)
                , resource(resource)
            {}
            ~LoadResourceJob() override = default;
            ResourceMap<K, V>* outMap;

            K key;
            std::shared_ptr<V> resource;
            void operator()() override {
                outMap->load(key, resource.get());
            }
        };

        ResourceMap(ResourceManager* owner) : owner(owner) {}

        std::shared_ptr<V> get(const K& key);

        void collectGarbage() {
            std::erase_if(resources, [](const auto& item) {
                // Remove anything where the only reference is our own
                return item.second.use_count() <= 1;
            });
        }
        void update();

        ~ResourceMap() {
            // This should delete everything
            collectGarbage();
            for (auto& ptr : resources) {
                // A resource not being deleted means that either we have a memory leak, or the resource manager wasn't destroyed last
                // The CPP standard specifies that members are destroyed in reverse order of declaration, and the resource manager is needed
                // for anything that uses it, so should be declared first
                std::cerr << "Resource " << ptr.first << " still had " << ptr.second.use_count() << " references at ResourceManager destruction" << std::endl;
            }
        }

    private:
        ResourceManager* owner;
        void load(const K& key, V* out);
        void upload(V* res);
        // Keep a strong reference and periodically check the use count
        // in order to keep resources that are frequently added/removed to the scene
        // from being loaded/unloaded constantly
        std::map<K, std::shared_ptr<V>> resources;

        std::vector<std::shared_ptr<V>> queuedUploads;
    };

    // Class to keep a cache of loaded resources, and to load them if not already
    // `get` only queues a resource to be loaded, therefore it is not safe to use
    // its value until the next call to `update`, where all queued loads are awaited.
    // This should be called immediately before rendering to ensure everything is available.
    //
    // Resources are loaded by worker threads, then uploaded by the main thread to the GPU
    //
    // A number of worker threads are allocated according to max(1, ceil(physicalThreads * threadMult))
    class ResourceManager
    {
    public:
        ResourceManager(GameTechRendererInterface* renderer, float threadMult);
        ~ResourceManager();

        // Run periodic garbage collection and upload all pending assets
        void update(float dt);
        // Free any resources that are not in active use
        // Called automatically every gcFrequency seconds
        void collectGarbage();

        GameTechRendererInterface* getRenderer() { return renderer; }

        //ResourceMap<std::string, Rendering::Texture>& getCubeMaps() { return cubeMaps; }
        ResourceMap<std::string, PlatformMesh>& getMeshes() { return meshes; }
        ResourceMap<std::string, PlatformTexture>& getTextures() { return textures; }
        ResourceMap<std::string, Material>& getMaterials() { return materials; }

        void addJob(std::unique_ptr<Job> job) {
            {
                std::lock_guard lock(jobsMtx);
                jobs.push_back(std::move(job));
                incompleteJobs++;
            }
            // Wake up a worker thread to complete the job
            jobsCv.notify_one();
        }
    protected:

        bool quitting = false;
        std::vector<std::thread> workerThreads;
        std::vector<std::unique_ptr<Job>> jobs;
        int incompleteJobs = 0;
        std::mutex jobsMtx;
        std::condition_variable jobsCv;

        void threadFunc();

        // Get a job once one is ready, or return nullptr when quitting
        std::unique_ptr<Job> getJob();

        void completeJob() {
            {
                std::lock_guard lock(jobsMtx);
                incompleteJobs--;
            }
            // Let the main thread know something's changed
            // notify_one cannot be used as it could be consumed by
            // a worker without anything to do, and would not fall back
            jobsCv.notify_all();
        }

        // Needed to upload platform-specific data to GPU
        GameTechRendererInterface* renderer;

        //ResourceMap<std::string, Rendering::Texture> cubeMaps;
        ResourceMap<std::string, PlatformMesh> meshes;
        ResourceMap<std::string, PlatformTexture> textures;
        ResourceMap<std::string, Material> materials;

        float gcFrequency = 30.0f;
        float timeSinceGc;
    };

    template <typename K, typename V>
    std::shared_ptr<V> ResourceMap<K, V>::get(const K& key)
    {
        auto it = resources.find(key);
        if (it == resources.end())
        {
            auto resource = std::make_shared<V>();
            queuedUploads.push_back(resource);
            auto job = std::make_unique<LoadResourceJob>(this, key, resource);
            owner->addJob(std::move(job));

            resources[key] = resource;
            return resource;
        }

        return it->second;
    }

    template <typename K, typename V>
    inline void ResourceMap<K, V>::update()
    {
        for (auto res : queuedUploads) {
            upload(res.get());
        }
        queuedUploads.clear();
    }
}
