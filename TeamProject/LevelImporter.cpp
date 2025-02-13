#include "LevelImporter.h"

using namespace NCL;
using namespace CSC8503;

using json = nlohmann::json;

LevelImporter::LevelImporter(ResourceManager* resourceManager, GameWorld* worldIn, btDiscreteDynamicsWorld* bulletWorldIn) {
    this->resourceManager = resourceManager;
    world = worldIn;
    bulletWorld = bulletWorldIn;
}

LevelImporter::~LevelImporter() {
    ClearObjects();
}

void LevelImporter::ClearObjects() {
    for (auto obj : objects) {
        delete obj;
    }
    objects.clear();
}

void LevelImporter::LoadLevel(int level) {
    ClearObjects();

    std::string filePath = Assets::LEVELDIR + "/level_" + std::to_string(level) + ".json";
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Could not open the file!" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        try {
            json j = json::parse(line);
            ObjectData* obj = new ObjectData();
            j.get_to(*obj);
            objects.push_back(obj);
        }
        catch (const json::exception& e) {
            std::cerr << "Error parsing JSON line: " << e.what() << std::endl;
            std::cerr << "Line content: " << line << std::endl;
        }
    }

    file.close();

    // Print out the list
    int count = 0;
    for (const auto& obj : objects) {
        count++;
        /*std::cout << "Object " << count << std::endl;
        std::cout << "Mesh: " << obj->meshName << ", Texture: " << obj->mainTextureName
            << ", Normal Texture: " << obj->normalTextureName << std::endl;
        std::cout << "Position: (" << obj->position.x() << ", " << obj->position.y() << ", " << obj->position.z() << ")"
            << " Rotation: (" << obj->rotation.x() << ", " << obj->rotation.y() << ", " << obj->rotation.z() << ", " << obj->rotation.w() << ")"
            << " Scale: (" << obj->scale.x() << ", " << obj->scale.y() << ", " << obj->scale.z() << ")\n"
            << "Collider Position: (" << obj->colliderPosition.x() << ", " << obj->colliderPosition.y() << ", " << obj->colliderPosition.z() << ")"
            << " Collider Scale: (" << obj->colliderScale.x() << ", " << obj->colliderScale.y() << ", " << obj->colliderScale.z() << ")\n\n";*/
            AddObjectToWorld(obj);
    }
}

void LevelImporter::AddObjectToWorld(ObjectData* data) {
    GameObject* cube = new GameObject();

    // Initializing variables for meshes and textures
    Mesh* selectedMesh = nullptr;
    bool isFloor = data->meshName == "corridor_walls_and_floor/Corridor_Floor_Basic";

	// If not a floor, apply an offset to move the floor up/down
    Vector3 position = data->position * scale;
    if (!isFloor) {
        position.y -= 8.0f;
    }
	cube->setInitialPosition(position);

    btVector3 eulerRotation = btVector3(data->rotation.getX(), data->rotation.getY(), data->rotation.getZ());
    if (data->meshName == "corridor_walls_and_floor/corridor_Wall_Straight_Mid_end_L") {
        eulerRotation.setX(eulerRotation.getX() - 90);
    }

    float pitchRadians = Maths::DegreesToRadians(eulerRotation.x());
    float yawRadians = Maths::DegreesToRadians(eulerRotation.y());
    float rollRadians = Maths::DegreesToRadians(eulerRotation.z());
    btQuaternion rotationQuat;
    rotationQuat.setEulerZYX(rollRadians, yawRadians, pitchRadians);
    cube->setInitialRotation(rotationQuat);
    cube->setRenderScale(data->scale* scale);


    btCollisionShape* boxShape;
    if (data->meshName == "corridor_walls_and_floor/corridor_Wall_Straight_Mid_end_L") {
        boxShape = new btBoxShape(btVector3(data->colliderScale.getX() / 2.0f, data->colliderScale.getZ() / 2.0f, data->colliderScale.getY() / 2.0f)* scale);
    }
    else {
        boxShape = new btBoxShape(btVector3(data->colliderScale.getX() / 2.0f, data->colliderScale.getY() / 2.0f, data->colliderScale.getZ() / 2.0f)* scale);
    }

    // The object is penetrating the floor a bit, so I reduced the bullet collision margin to avoid sinking in the floor
    boxShape->setMargin(0.01f);

	btCompoundShape* compoundShape = new btCompoundShape();
    btTransform colliderOffset;
	colliderOffset.setIdentity();


    colliderOffset.setOrigin(btVector3(data->colliderPosition.getX(), -data->colliderPosition.getY() * 6, data->colliderPosition.getZ()) * (scale / 2));

    //if (!isFloor) {
    //    colliderOffset.setOrigin(btVector3(0, (data->colliderPosition.getY() / 2.0f * scale) + 27, data->colliderScale.getZ() - 10.5));
    //}
    //else {
    //    colliderOffset.setOrigin(btVector3(0, data->colliderPosition.getY(), 0));  // No offset for floor
    //}


	compoundShape->addChildShape(colliderOffset, boxShape);
    // Setting the physics object for the cube
    cube->SetPhysicsObject(new PhysicsObject(cube));
    cube->GetPhysicsObject()->InitBulletPhysics(bulletWorld, compoundShape, 0, true);
    // Setting render object

    // TODO: Use null instead of magic
    // TODO: Include extensions
    auto texture = data->mainTextureName == "No Texture" ? nullptr : resourceManager->getTextures().get(data->mainTextureName + ".tga");
    auto normalTexture = data->normalTextureName == "No Normal Texture" ? nullptr : resourceManager->getTextures().get(data->normalTextureName + ".tga");
    cube->SetRenderObject(new RenderObject(cube, resourceManager->getMeshes().get(data->meshName + ".msh"), texture, resourceManager->getShaders().get(Shader::Default), normalTexture));
    world->AddGameObject(cube);
    cube->setIsFloor(true);
}
