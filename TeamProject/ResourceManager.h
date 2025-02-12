#pragma once

#include <map>
#include <string>
#include <memory>
#include <iostream>

// Forward declarations won't work here, as we need the full declaration to generate templates
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"

namespace NCL::CSC8503 {

    // A map of resources, where the key is used to load the resource
    // `K` is the type of the key, which must be comparable
    // `V` is the type of the resource, which must have a constructor that takes a reference to a `K`
    // The resource is constructed on the first call to `get` for a given key, and destroyed when the last
    // reference other than the one in the map is destroyed
    template <typename K, typename V>
    class ResourceMap
    {
    public:
        std::shared_ptr<V> get(const K& key)
        {
            auto it = resources.find(key);
            // Attempting to lock an expired weak pointer will return a nullptr
            // We don't use expired() here, as it would introduce a TOCTOU
            std::shared_ptr<V> ptr = it != resources.end() ? it->second.lock() : nullptr;

            if (ptr == nullptr)
            {
                auto resource = std::make_shared<V>(key);
                resources.emplace(key, resource);
                return resource;
            }

            return ptr;
        }

        ~ResourceMap() {
            for (auto& ptr : resources) {
                if (!ptr.second.expired()) {
                    // A resource not being deleted means that either we have a memory leak, or the resource manager wasn't destroyed last
                    // The CPP standard specifies that members are destroyed in reverse order of declaration, and the resource manager is needed
                    // for anything that uses it, so should be declared first
                    std::cerr << "Resource " << ptr.first << " not deleted before ResourceManager destruction" << std::endl;
                }
            }
        }
    private:
        // The resource manager is expected to last for the lifetime of the program,
        // so we use weak pointers to allow resources to be deleted when no longer needed
        // and reloaded when they are needed again
        // A more advanced implementation might cache resources that might be needed again
        // or allow for them to be loaded asynchronously (OpenGL makes this difficult, as
        // we'd have to queue operations for the main thread)
        std::map<K, std::weak_ptr<V>> resources;
    };

    // Class to keep a cache of loaded resources, and to load them if not already
    // Methods may affect the OpenGL state, so should be called from the main thread
    // and not during rendering
    class ResourceManager
    {
    public:
        ResourceManager();

        ResourceMap<std::string, Rendering::Texture>& getTextures() { return textures; }
        //ResourceMap<std::string, Rendering::Texture>& getCubeMaps() { return cubeMaps; }
        ResourceMap<Rendering::Shader::Key, Rendering::Shader>& getShaders() { return shaders; }
        ResourceMap<std::string, Rendering::Mesh>& getMeshes() { return meshes; }
    protected:
        ResourceMap<std::string, Rendering::Texture> textures;
        //ResourceMap<std::string, Rendering::Texture> cubeMaps;
        ResourceMap<Rendering::Shader::Key, Rendering::Shader> shaders;
        ResourceMap<std::string, Rendering::Mesh> meshes;
    };

}