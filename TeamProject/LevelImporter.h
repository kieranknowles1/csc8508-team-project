#pragma once

#include "GameObject.h"
#include "Assets.h"
#include <fstream>
#include <btBulletDynamicsCommon.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <vector>
#include <string>
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"

#endif
#include "Vector.h"
#include "Matrix.h"
#include "Camera.h"
#include "Controller.h"
#include "StateGameObject.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"
#include "BulletDebug.h"
#include "PlayerObject.h"

using json = nlohmann::json;

// Custom from_json functions for btVector3 and btQuaternion
inline void from_json(const json& j, btVector3& vec) {
    vec.setX(j.at("x").get<float>());
    vec.setY(j.at("y").get<float>());
    vec.setZ(j.at("z").get<float>());
}


// Define the ObjectData class
class ObjectData {
public:
    // Object transform
    btVector3 position;
    btVector3 rotation;
    btVector3 scale;

    // Collider transform
    btVector3 colliderPosition;
    btVector3 colliderScale;

    // Other
    std::string meshName;
    std::string mainTextureName;
    std::string normalTextureName;

    friend void from_json(const json& j, ObjectData& obj) {
        j.at("position").get_to(obj.position);
        j.at("rotation").get_to(obj.rotation);
        j.at("scale").get_to(obj.scale);
        j.at("colliderPosition").get_to(obj.colliderPosition);
        j.at("colliderScale").get_to(obj.colliderScale);
        j.at("meshName").get_to(obj.meshName);
        j.at("mainTextureName").get_to(obj.mainTextureName);
        j.at("normalTextureName").get_to(obj.normalTextureName);
    }
};

using namespace NCL;
using namespace CSC8503;

class LevelImporter {
public:
    LevelImporter(GameTechRenderer* rendererIn, GameWorld* worldIn, btDiscreteDynamicsWorld* bulletWorldIn);
    ~LevelImporter();

    void LoadLevel(int level);
    void AddObjectToWorld(ObjectData* data);
    void ClearObjects();

private:
    GameTechRenderer* renderer;
    GameWorld* world;
    btDiscreteDynamicsWorld* bulletWorld;
    std::vector<ObjectData* > objects;

    Mesh* cubeMesh = nullptr;
    Texture* basicTex = nullptr;
    Texture* wallTex = nullptr;
    Shader* basicShader = nullptr;
    Mesh* wallSection = nullptr;
    Mesh* floorSection = nullptr;

    float scale = 10.0;

};
