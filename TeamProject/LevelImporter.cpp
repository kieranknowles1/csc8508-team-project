#include "LevelImporter.h"

using namespace NCL;
using namespace CSC8503;

using json = nlohmann::json;

LevelImporter::LevelImporter(GameTechRenderer* rendererIn, GameWorld* worldIn, btDiscreteDynamicsWorld* bulletWorldIn) {
    renderer = rendererIn;
    world = worldIn;
    bulletWorld = bulletWorldIn;

    cubeMesh = renderer->LoadMesh("Cube.msh");
    basicTex = renderer->LoadTexture("checkerboard.png");
    basicShader = renderer->LoadShader("scene.vert", "scene.frag");
    wallSection = renderer->LoadMesh("Corridor_Meshes/corridor_Wall_Straight_Mid_end_L.msh");
    floorSection = renderer->LoadMesh("Corridor_Meshes/Corridor_Floor_Basic.msh");
    wallTex = renderer->LoadTexture("Corridor_Textures/corridor_wall_c.tga");
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
    Texture* selectedTex = basicTex;
    bool isFloor = false;

	// This mesh names have been used to identify the objects in the level
	// and set the offset for the wall only as the collider is not aligned with the mesh
    if (data->meshName == "corridor_walls_and_floor:corridor_Wall_Straight_Mid_end_L") {
        selectedMesh = wallSection;
        selectedTex = wallTex;
    }
    else if (data->meshName == "corridor_walls_and_floor:Corridor_Floor_Basic") {
        selectedMesh = floorSection;
        selectedTex = basicTex;
		isFloor = true;
    }
    else {
        std::cerr << "NO MESH FOUND FOR LEVEL OBJECT" << std::endl;
        return;
    }

	// If not a floor, apply an offset to move the floor up/down
    Vector3 position = data->position * scale;
    if (!isFloor) {
        position.y -= 8.0f;
    }
	cube->setInitialPosition(position);

    btVector3 eulerRotation = btVector3(data->rotation.getX(), data->rotation.getY(), data->rotation.getZ());
    if (selectedMesh == wallSection) {
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
    if (selectedMesh == wallSection) {
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

    // Check if it's a floor or not and apply the offset to the collider upward to align it with the mesh
    if (!isFloor) {
        colliderOffset.setOrigin(btVector3(0, (data->colliderPosition.getY() / 2.0f * scale) + 27, data->colliderScale.getZ() - 10.5));
    }
    else {
        colliderOffset.setOrigin(btVector3(0, data->colliderPosition.getY(), 0));  // No offset for floor
    }

	compoundShape->addChildShape(colliderOffset, boxShape);

    // Setting the physics object for the cube
    cube->SetPhysicsObject(new PhysicsObject(cube));
    cube->GetPhysicsObject()->InitBulletPhysics(bulletWorld, compoundShape, 0, true);

    cube->SetRenderObject(new RenderObject(cube, selectedMesh, selectedTex, basicShader));
    world->AddGameObject(cube);

    cube->setIsFloor(true);
}
