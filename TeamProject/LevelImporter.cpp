#include "LevelImporter.h"
#include "PointLight.h"
#include "Respawn.h"
#include <nlohmann/json.hpp>

using namespace NCL;
using namespace CSC8503;

using json = nlohmann::json;

// Custom from_json functions for btVector3 and btQuaternion
inline void from_json(const json& j, btVector3& vec) {
    vec.setX(j.at("x").get<float>());
    vec.setY(j.at("y").get<float>());
    vec.setZ(j.at("z").get<float>());
}

inline void from_json(const json& j, btQuaternion& quat) {
    quat.setX(j.at("x").get<float>());
    quat.setY(j.at("y").get<float>());
    quat.setZ(j.at("z").get<float>());
    quat.setW(j.at("w").get<float>());
}

void from_json(const json& j, ObjectData& obj) {
    j.at("position").get_to(obj.position);
    j.at("rotation").get_to(obj.rotation);
    j.at("scale").get_to(obj.scale);
    j.at("colliderPosition").get_to(obj.colliderPosition);
    j.at("colliderScale").get_to(obj.colliderScale);
    j.at("meshName").get_to(obj.meshName);
    // TODO: This should be done during export
    auto colon = obj.meshName.find(":");
    if (colon != std::string::npos) {
        obj.meshName.replace(colon, 1, "/");
    }
    j.at("mainTextureName").get_to(obj.mainTextureName);
    j.at("normalTextureName").get_to(obj.normalTextureName);
    j.at("type").get_to(obj.type);
}

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
        json j = json::parse(line);
        ObjectData* obj = new ObjectData();
        j.get_to(*obj);
        objects.push_back(obj);
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

    // Immediately free anything not needed by the new scene that was loaded for the old one
    resourceManager->collectGarbage();
    std::cout << "Loaded level " << level << "; Contained " << count << " GameObjects" << std::endl;
}

void LevelImporter::AddObjectToWorld(ObjectData* data) {
    GameObject* cube = new GameObject();

    // Initializing variables for meshes and textures
    Mesh* selectedMesh = nullptr;
    // TODO: Don't hard code this
    bool isFloor = data->meshName == "corridor_walls_and_floor/Corridor_Floor_Basic";
	cube->setInitialPosition(data->position * scale);

    cube->setInitialRotation(data->rotation);
    cube->setRenderScale(data->scale* scale);


    // Divide by 2 for half-size
    btCompoundShape* compoundShape = nullptr;
    bool hasCollision = (data->colliderScale != btVector3(0,0,0));
    if (hasCollision) {
        compoundShape = new btCompoundShape();
        compoundShape->setUserIndex((int)PhysicsObject::ShapeType::Compound);
        btCollisionShape* boxShape = new btBoxShape(data->colliderScale * scale* data->scale / 2.0f);
        btTransform colliderOffset;
        colliderOffset.setIdentity();
        colliderOffset.setOrigin(data->colliderPosition * scale);
        compoundShape->addChildShape(colliderOffset, boxShape);
    }
    cube->SetPhysicsObject(new PhysicsObject(cube));
    cube->GetPhysicsObject()->InitBulletPhysics(bulletWorld, compoundShape, 0, hasCollision);


    // Setting render object
    // TODO: Include file extensions
    auto mainTexture = [&](const std::string& tex) {
        return tex.empty() ? resourceManager->getTextures().get("checkerboard.png") : resourceManager->getTextures().get(tex + ".jpg");
    };
    auto normalTexture = [&](const std::string& tex) {
        return tex.empty() ? nullptr : resourceManager->getTextures().get(tex + ".png");
        };
    cube->SetRenderObject(new RenderObject(
        cube,
        resourceManager->getMeshes().get(data->meshName + ".msh"),
        mainTexture(data->mainTextureName),
        normalTexture(data->normalTextureName)
    ));
    world->AddGameObject(cube);
    cube->GetRenderObject()->SetTexRepeating(true);//sets texture to repeat and scale
    cube->setType(data->type);
    HandleTypes(cube);
}

btVector4 LevelImporter::LightColour() {
    switch (lightCount) {
    case 0: return btVector4(1.0f, 0.0f, 0.0f, 1.0f); // Red
    case 1: return btVector4(0.0f, 1.0f, 0.0f, 1.0f); // Green
    case 2: return btVector4(0.0f, 0.0f, 1.0f, 1.0f); // Blue
    case 3: return btVector4(0.0f, 1.0f, 1.0f, 1.0f); // Cyan
    case 4: return btVector4(1.0f, 0.0f, 1.0f, 1.0f); // Magenta
    case 5: return btVector4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
    default: return btVector4(1.0f, 1.0f, 1.0f, 1.0f); // White (fallback, shouldn't be hit)
    }
}


void LevelImporter::HandleTypes(GameObject* obj) {
    btVector4 colourLight = LightColour();
    switch (obj->getType())
    {

    case GameObject::Type::JumpPad:
        obj->GetRenderObject()->SetColour(Vector4(0.3f, 0.3f, 0.3f, 1));
        obj->GetRenderObject()->SetTexScaleMultiplier(0.0025f);
        break;
    case GameObject::Type::Slime:
        obj->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
        obj->GetRenderObject()->SetTexScaleMultiplier(0.0025f);
        break;
    case GameObject::Type::Ice:
        obj->GetRenderObject()->SetColour(Vector4(1, 2.0f, 2.0f, 1));
        break;
    case GameObject::Type::SlimeCastle:
        obj->GetRenderObject()->SetColour(Vector4(0.8f, 0.8f, 0.8f, 1));
        obj->GetRenderObject()->SetTexScaleMultiplier(0.0035f);
        break;
    case GameObject::Type::Centre:
        obj->GetRenderObject()->SetTexScaleMultiplier(0.0025f);
        break;


    case GameObject::Type::PointLight:
        obj->GetRenderObject()->setMaterial(nullptr);
        world->AddPointLight(new PointLight(obj->GetPhysicsObject()->GetRigidBody()->getWorldTransform().getOrigin(), 950,1, colourLight));
        colourLight *= 10;
        colourLight.setW(1.0f);
        obj->GetRenderObject()->SetColour(colourLight);
        lightCount++;
        break;
    case GameObject::Type::RespawnPoint:
    {
        btMatrix3x3 rotationMatrixCam(obj->GetTransform().getRotation());
        btVector3 upwards = rotationMatrixCam * btVector3(0, 1, 0); // Apply rotation to the offset
        RespawnPoint* respawnPoint = new RespawnPoint(obj->GetTransform().getOrigin(),upwards);
        Respawn::GetInstance()->InsertRespawn(respawnPoint);
        break;
    }
    default:
        break;
    }
}
