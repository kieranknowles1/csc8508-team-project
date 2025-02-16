#pragma once

#include "GameObject.h"
#include "Assets.h"
#include <fstream>
#include <btBulletDynamicsCommon.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <vector>
#include <string>
#include "Vector.h"
#include "Matrix.h"
#include "Camera.h"
#include "Controller.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"
#include "BulletDebug.h"
#include "PlayerObject.h"
#include "ResourceManager.h"

using json = nlohmann::json;

// Define the ObjectData class
class ObjectData {
public:
    // Object transform
    btVector3 position;
    btQuaternion rotation;
    btVector3 scale;

    // Collider transform
    btVector3 colliderPosition;
    btVector3 colliderScale;

    // Other
    std::string meshName;
    std::string mainTextureName;
    std::string normalTextureName;
};

using namespace NCL;
using namespace CSC8503;

class LevelImporter {
public:
    LevelImporter(ResourceManager* resourceManager, GameWorld* worldIn, btDiscreteDynamicsWorld* bulletWorldIn);
    ~LevelImporter();

    void LoadLevel(int level);
    void AddObjectToWorld(ObjectData* data);
    void ClearObjects();

private:
    ResourceManager* resourceManager;
    GameWorld* world;
    btDiscreteDynamicsWorld* bulletWorld;
    std::vector<ObjectData*> objects;

    float scale = 20.0;

};
