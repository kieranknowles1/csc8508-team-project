#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"
#include "AudioEngine.h"
#include "GameTechRendererInterface.h"
#include "BulletDebug.h"
#include "Multiplayer/GamePackets.hpp"
#include "Multiplayer/GamePacketHandlers.hpp"
#include "Multiplayer/Server.hpp"
#include <CSC8503CoreClasses/Debug.h>
#include "Colors.h"
#include "Shoot.h"
#include "Health.h"
#include "MeshAnimation.h" //temporarily added for testing
#include "Score.h"

#include "Window.h"
#include "Config.h"

using namespace NCL;
using namespace CSC8503;
using namespace Lobbies;

TutorialGame* TutorialGame::instance = nullptr;

TutorialGame::TutorialGame(GameTechRendererInterface* renderer, Controller* controller, Config& config)
    : renderer(renderer)
    , controller(controller)
    , config(config)
{
    assert(instance == nullptr && "TutorialGame must be unique");
    instance = this;

    stateMutex = new std::shared_mutex();
    world = std::make_unique<GameWorld>();
    renderer->setCamera(&world->GetMainCamera());

    /* Initializing the Bullet Physics World here as it should be done before Initialize the NCL framework's PhysicsSystem */
    //InitBullet(); //bullet is initialised in initialiseAssets already
    world->GetMainCamera().SetController(controller);
    mainCamera = &world->GetMainCamera();

    resourceManager = std::make_unique<ResourceManager>(renderer, config.get<float>("resourceThreadMult"));
    new Shoot(); //Shoot and Respawn have new before them but are not being deleted to my knowledge
    new Respawn();
    //audioEngine.Init();
    audioEngine.Init();
    audioEngine.LoadSound("HeartbeatLoop.wav", false, true, false);
    audioEngine.LoadSound("JumpPad.wav", true, false, false);
    InitialiseAssets();
    InitCamera();
    InitWorld();

}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes,
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
    defaultTexture = resourceManager->getTextures().get("checkerboard.png");
    paintballTexture = resourceManager->getTextures().get("paintball_basecolor.png");
}

TutorialGame::~TutorialGame()	{
    DestroyBullet();
    audioEngine.Shutdown();

    delete server;
    delete stateMutex;
    instance = nullptr;
}

static bool BulletRaycast(btDynamicsWorld* world, const btVector3& start, const btVector3& end, btCollisionWorld::ClosestRayResultCallback& resultCallback) {
    world->rayTest(start, end, resultCallback);
    return resultCallback.hasHit();
}

void TutorialGame::UpdateGame(float dt) {
    profiler.beginFrame();
    profiler.startSection("Physics");

    // Old
    //int substeps = std::floor(dt / PHYSICS_PERIOD);
    //int steps = bulletWorld->stepSimulation(dt , substeps, PHYSICS_PERIOD);

    ////New
    int substeps = 0;
    //float maxDt = btMin(PHYSICS_PERIOD, dt);
    int steps = bulletWorld->stepSimulation(dt, substeps, PHYSICS_PERIOD);

    profiler.startSection("Network Updates");
    if (server != nullptr) {
        std::unique_lock ticklock = server->LockTick();

        float tickProgress = server->GetTickProgress();
        world->OperateOnContents([&](GameObject* obj) {
            if (!obj->IsNetworked()) return;

            ServerObject* netObj = (ServerObject*)obj;

            if (server->IsOwnerOf(netObj)) netObj->UpdateWorldState();
            else netObj->UpdateFromWorldState(dt);
            });
    }

    profiler.startSection("Update World");

    if(spGameController) spGameController->Update(dt);

    UpdateKeys();
    world->UpdateWorld(dt);
    profiler.startSection("Check Collisions");
    // Check for collisions
    CheckCollisions();

    profiler.startSection("Update Audio");
    audioEngine.Update(&world->GetMainCamera());

    clearGraveyard();
    profiler.startSection("Prepare Render");
    bulletWorld->debugDrawWorld();

    profiler.startSection("Render Decals");
    // Fade decal after sometime - @Kieran: Didn't forget to call the Update function this time :)
    renderer->GetDecalSystem().Update(dt);

    profiler.endFrame();
    if (showProfiling) {
        profiler.printTimes();
    }

    //post processing time variable effect:
    pulse += dt;
    renderer->SetVignettePulse(pulse);

    renderer->SetDelta(dt);
}

void TutorialGame::UpdatePlayer(float dt, bool camOnly) {
    if (player->GetHealthAttrib()->GetHealthState() == AliveState::DEAD) {
        player->GetHealthAttrib()->AddDeath();

        numDeaths = player->GetHealthAttrib()->DeathCount();

        if (gameMode == GameMode::SINGLEPLAYER && numDeaths > 2) {
            spEnd = true;
        }

        Respawn* instance = Respawn::GetInstance();
        RespawnPoint* respawn;

        if (player->GetOwner()) respawn = instance->GetRandomRespawn(player->GetOwner()->GetUserID() - 1);
        else respawn = instance->GetRandomRespawn(1);

        std::cout << "Here" << std::endl;
        btTransform& transform = player->GetPhysicsObject()->GetRigidBody()->getWorldTransform();

        transform.setOrigin(respawn->position);
        player->setUpDirection(respawn->orientation);
        playerController->setYaw(respawn->yaw);

        player->setCollided(0);
        player->GetHealthAttrib()->Respawn();

    }

    // Display Score if networked.
    if (server != nullptr) {
        std::string scoreString = "Score: " + std::to_string(static_cast<int>(player->GetScoreAttrib()->GetScore()));
        Debug::Print(scoreString, { 0.05, 0.1 });
    }

    // Press F for freeCam, press G for thirdPerson
    if (freeCam) {
        //freeCam Movement
        world->GetMainCamera().UpdateCamera(dt * 20, true);
    }
    else {
 
        //player Movement
        if (camOnly) {
            playerController->UpdateCamOnly();
        }
        else {
            world->GetMainCamera().UpdateCamera(dt, false);
            playerController->UpdateMovement(dt);
        }
 
        if (thirdPerson) {
            ThirdPersonControls();
        }
    }
}

void TutorialGame::UpdateKeys() {
    using enum Controller::DigitalControl;
    if (controller->GetDigital(DebugReloadWorld)) {
        InitWorld(); //We can reset the simulation at any time with F1
    }
    if (controller->GetDigital(DebugBulletOverlay)) {
        bulletDebug->toggle();
    }
    if (controller->GetDigital(DebugShowProfiling)) {
        showProfiling = !showProfiling;
    }
    if (controller->GetDigital(DebugFreeCam)) {
        freeCam = !freeCam;
    }
    if (controller->GetDigital(DebugRespawnRandom)) {
        RespawnPoint* respawnPoint = Respawn::GetInstance()->GetRandomRespawn(player->GetWorldID());
        playerController->setYaw(respawnPoint->yaw);
        player->GetPhysicsObject()->GetRigidBody()->getWorldTransform().setOrigin(respawnPoint->position);
        player->setUpDirection(respawnPoint->orientation);
        player->resetCollisionType();
        player->setCollided(0);
    }


    if (playerController) {
        if (controller->GetDigital(ThirdPerson)) {
            thirdPerson = !thirdPerson;
            playerController->SetThirdPerson(thirdPerson);
        }
        if (controller->GetDigital(WorldRollRight)) {
            playerController->rollRight();
        }
        if (controller->GetDigital(WorldRollLeft)) {
            playerController->rollLeft();
        }
        if (controller->GetDigital(WorldPitchUp)) {
            playerController->pitchUp();
        }
        if (controller->GetDigital(WorldPitchDown)) {
            playerController->pitchDown();
        }
    }
}

void TutorialGame::ThirdPersonControls() {
    btTransform transformPlayer = player->GetPhysicsObject()->GetRigidBody()->getWorldTransform();
    btQuaternion playerRotation1(btVector3(0, 1, 0), Maths::DegreesToRadians(playerController->getYaw()));
    btMatrix3x3 rotationMatrix(player->getCamOffset() * playerRotation1);
    btVector3 forward = rotationMatrix * btVector3(0,0,-1);
    btVector3 upwards = rotationMatrix * btVector3(0, 1, 0);
    float camHeight = 35.0f;
    float camDist = -100.0f;
    btVector3 cameraOffset = (forward.normalize() * camDist) + (upwards.normalize() * camHeight);
    btVector3 cameraPosition = transformPlayer.getOrigin() + cameraOffset;
    mainCamera->SetPosition(cameraPosition);
    mainCamera->SetPitch(-15.0f);
}


void TutorialGame::CheckCollisions()
{
    // Checking for collisions using Bullet's collision detection system
    // Bullet already keeps track of all the objects that are colliding with each other
    // So, we don't need to check for collisions manually

    //world->OperateOnContents([&](GameObject* obj) {
    //	obj->GetPhysicsObject()->CheckCollisions(bulletWorld);
    //	});

    btDispatcher* dispatcher = bulletWorld->getDispatcher();
    int numManifolds = dispatcher->getNumManifolds();

    for (int i = 0; i < numManifolds; i++) {
        // The UpdateGame loop may be using a faster an outdated number of manifolds
        // So, we need to check if the index is still valid
        if (i >= dispatcher->getNumManifolds()) {
            break;
        }

        // Get the contact manifold
        btPersistentManifold* contactManifold = dispatcher->getManifoldByIndexInternal(i);

        // Get the collision objects from the contact manifold
        const btCollisionObject* objectA = contactManifold->getBody0();
        const btCollisionObject* objectB = contactManifold->getBody1();

        // Get the GameObjects from the collision objects
        const GameObject* gameObjectA = static_cast<const GameObject*>(objectA->getUserPointer());
        const GameObject* gameObjectB = static_cast<const GameObject*>(objectB->getUserPointer());

        // Check if the GameObjects are valid
        if (gameObjectA && gameObjectB) {
            gameObjectA->GetPhysicsObject()->CheckCollisions(bulletWorld);
            gameObjectB->GetPhysicsObject()->CheckCollisions(bulletWorld);
        }
    }
}

void TutorialGame::clearGraveyard() {
    for (auto obj : earlyGraveyard) {
        // Prevents physics, OnCollisionExit will trigger next frame
        if (obj->GetPhysicsObject()) {
            bulletWorld->removeRigidBody(obj->GetPhysicsObject()->GetRigidBody());
        }
        // Prevents OnUpdate and render
        world->RemoveGameObject(obj);
    }
    // Deleting a GameObject may trigger other GameObjects to be deleted,
    // make sure these go through the full process
    auto addLateQueue = std::move(earlyGraveyard);
    for (auto obj : lateGraveyard) {
        // References should have been cleaned up by now
        // May not be strictly necessary to delay this, but it's
        // not worth the risk for a micro-optimization
        delete obj;
    }
    // Move earlyGraveyard to lateGraveyard, clear lateGraveyard
    lateGraveyard = std::move(addLateQueue);
}


void TutorialGame::InitCamera() {
    mainCamera->SetFieldOfVision(90);
    world->GetMainCamera().SetNearPlane(1.75f);
    world->GetMainCamera().SetFarPlane(5000.0f);
    world->GetMainCamera().SetPitch(-15.0f);
    world->GetMainCamera().SetYaw(315.0f);
    world->GetMainCamera().SetPosition(Vector3(-60, 40, 60));
}

void TutorialGame::DestroyBullet() {
    // TODO: These could all be unique_ptr
    delete bulletWorld;
    delete bulletDebug;
    delete solver;
    delete dispatcher;
    delete collisionConfig;
    delete broadphase;

    bulletWorld = nullptr; bulletDebug = nullptr; solver = nullptr;
    dispatcher = nullptr; collisionConfig = nullptr; broadphase = nullptr;
}

/* Bullet Physics world has been initialized here */
void TutorialGame::InitBullet() {
    broadphase = new btDbvtBroadphase();
    collisionConfig = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfig);
    solver = new btSequentialImpulseConstraintSolver();

    bulletWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfig);
    bulletWorld->setGravity(btVector3(0.0f, 0.0f, 0.0f));
    bulletDebug = new BulletDebug();
    bulletWorld->setDebugDrawer(bulletDebug);
}

void TutorialGame::LoadWorldFromFile(int levelNum) {
    ClearWorld();
    InitWorld();

    LevelImporter levelImporter(resourceManager.get(), world.get(), bulletWorld);
    levelImporter.LoadLevel(levelNum);

}


void TutorialGame::ClearWorld() {
    DestroyBullet();
    world->ClearAndErase();
    renderer->GetDecalSystem().ClearDecalsFromWorld();
    renderer->ClearUIElemets();
    Respawn::GetInstance()->ClearPlayers();
    earlyGraveyard.clear();
    lateGraveyard.clear();
}

void TutorialGame::InitWorld() {

	InitBullet();
    //audioEngine.Init();

}

PlayerObject* TutorialGame::InitPlayer(btVector3 position, btVector3 upDir, bool mainPlayer) {
    PlayerObject* newPlayer = AddPlayerCapsuleToWorld(position, 20.0f, 8.5f, 10.0f, mainPlayer);
    // Keep us from clipping when falling too fast
    newPlayer->GetPhysicsObject()->GetRigidBody()->setCcdMotionThreshold(1.0f);
    newPlayer->GetPhysicsObject()->GetRigidBody()->setCcdSweptSphereRadius(0.4f);
    newPlayer->GetPhysicsObject()->GetRigidBody()->setAngularFactor(0);
    newPlayer->GetPhysicsObject()->GetRigidBody()->setFriction(0.0f);
    newPlayer->GetPhysicsObject()->GetRigidBody()->setDamping(0.0, 0);
    newPlayer->GetPhysicsObject()->GetRigidBody()->setGravity({0, 0, 0});

    newPlayer->GetRenderObject()->SetColour(Vector4(playerColour));
    newPlayer->setUpDirection(upDir);
    newPlayer->setRenderer(renderer);
    newPlayer->setType(GameObject::Type::Player);

    newPlayer->setRenderer(renderer);
    Respawn::GetInstance()->InsertPlayerObj(newPlayer);
    return newPlayer;
}

GameObject* TutorialGame::AddGunToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, bool hasCollision)
{
    GameObject* gun = new GameObject();

    // Setting the transform properties for the gun
    gun->setInitialPosition(position);
    gun->setRenderScale(dimensions);

    btCollisionShape* shape = new btBoxShape(btVector3(dimensions.x / 2.0f, dimensions.y / 2.0f, dimensions.z / 2.0f));

    // Setting the physics object for the gun
    gun->SetPhysicsObject(new PhysicsObject(gun));

    // Initialize Bullet physics for the gun
    gun->GetPhysicsObject()->InitBulletPhysics(bulletWorld, shape, inverseMass, hasCollision);

    // Setting render object
    gun->SetRenderObject(new RenderObject(gun, resourceManager->getMeshes().get("VD_Raygun_Cartoony_Rigged1.msh"), resourceManager->getMaterials().get("VD_Raygun_Cartoony_Rigged1.mat")));
    gun->setType(GameObject::Type::Gun);

    gun->setType(GameObject::Type::Gun);

    world->AddGameObject(gun);

    return gun;
}

/* Adding an object to test the bullet physics */
GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, bool hasCollision) {
    GameObject* cube = new GameObject();

    // Setting the transform properties for the cube
    cube->setInitialPosition(position);
    cube->setRenderScale(dimensions);

    // TODO : Set the orientation of the cube
    //cube->SetOrientation(rotation);

    // Creating Bullet collision shape
    // Note: The scale of the cube is set when creating the collision shape, When the collision shapes are created,
    // so the size of the collision shape acts as the scale of the object
    btCollisionShape* shape = new btBoxShape(btVector3(dimensions.x / 2.0f, dimensions.y / 2.0f, dimensions.z / 2.0f));
    // The object is penetrating the floor a bit, so I reduced the bullet collison margin to avoid sinking in the floor
    shape->setMargin(0.01f);

    // Setting the physics object for the cube
    cube->SetPhysicsObject(new PhysicsObject(cube));

    // Initialize Bullet physics for the cube
    // WTF: Setting shape to nullptr causes camera stutter
    cube->GetPhysicsObject()->InitBulletPhysics(bulletWorld, shape, inverseMass, hasCollision);

    // Setting render object
    cube->SetRenderObject(new RenderObject(cube, resourceManager->getMeshes().get("Cube.msh"), defaultTexture));
    cube->GetRenderObject()->SetTexRepeating(true); //scale texture (no stretching)

    world->AddGameObject(cube);

    return cube;
}

PlayerObject* TutorialGame::AddPlayerCapsuleToWorld(const Vector3& position, float height, float radius, float inverseMass,bool mainPlayer) {
    PlayerObject* player = new PlayerObject();

    // Setting the transform properties for the capsule
    player->setInitialPosition(position);
    player->setRenderScale(mainPlayer ? Vector3(0,0,0) : Vector3(radius * 3.0f, height*1.1f, radius * 3.0f));

    // Creating a Bullet collision shape for the capsule
    btCollisionShape* playerShape = new btCapsuleShape(radius, height);

    // Setting the render object for the capsule
    player->SetRenderObject(new RenderObject(player, resourceManager->getMeshes().get("RacerGuy/RacerGuy2.msh"), resourceManager->getMaterials().get("RacerGuy.mat"))); //defaultTexture
    player->CreateAnimationObject();
    player->CorrectAnimation();

    // Setting the physics object for the capsule
    player->SetPhysicsObject(new PhysicsObject(player));

    // Initializing the physics object for the capsule
    player->GetPhysicsObject()->InitBulletPhysics(bulletWorld, playerShape, inverseMass);
    GameObject* newGun = AddGunToWorld(Vector3(-900, 20, 40), Vector3(3, 3, 3), 0, false);
    player->setGun(newGun);
    world->AddGameObject(player);

    // Insert a laser object the player will use.
    LaserObject* laser = new LaserObject(player);
    laser->SetThickness(0.25f);
    world->AddGameObject(laser);

    player->SetLaser(laser);
    return player;
}

GameObject* TutorialGame::AddCapsuleToWorld(const Vector3& position, float height, float radius, float inverseMass) {
    GameObject* capsule = new GameObject();

    // Setting the transform properties for the capsule
    capsule->setInitialPosition(position);
    capsule->setRenderScale(Vector3(radius * 2, height, radius * 2));

    // TODO: Set the orientation of the capsule
    //capsule->SetOrientation(rotation);

    // Creating a Bullet collision shape for the capsule
    btCollisionShape* shape = new btCapsuleShape(radius, height);

    // Setting the render object for the capsule
    capsule->SetRenderObject(new RenderObject(capsule, resourceManager->getMeshes().get("Capsule.msh"), defaultTexture));
    // Setting the physics object for the capsule
    capsule->SetPhysicsObject(new PhysicsObject(capsule));

    // Initializing the physics object for the capsule
    capsule->GetPhysicsObject()->InitBulletPhysics(bulletWorld, shape, inverseMass);

    world->AddGameObject(capsule);

    return capsule;
}

GameObject* TutorialGame::AddInfinitePlaneToWorld(const Vector3& position, const Vector3& normal, float planeConstant) {
    GameObject* plane = new GameObject();

    // Set the transform properties for the plane
    plane->setInitialPosition(position);

    // Create Bullet collision shape for an infinite plane
    btCollisionShape* shape = new btStaticPlaneShape(btVector3(normal.x, normal.y, normal.z), planeConstant);

    // Set the render object
    plane->SetRenderObject(new RenderObject(plane, resourceManager->getMeshes().get("Plane.msh"), defaultTexture));
    plane->GetRenderObject()->SetTexRepeating(true); //scale texture

    // Set the physics object
    plane->SetPhysicsObject(new PhysicsObject(plane));

    // Reduce collision margin (though planes typically don't use it)
    shape->setMargin(0.01f);

    // Initialize Bullet physics for the plane
    plane->GetPhysicsObject()->InitBulletPhysics(bulletWorld, shape, 0.0f); // Static plane, so inverse mass = 0

    world->AddGameObject(plane);

    return plane;
}


/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position, const Vector3& size, const Vector3& rotation) {
    GameObject* floor1 = AddCubeToWorld(position, size, 0);
    btVector3 eulerRotation = rotation;
    float pitchRadians = Maths::DegreesToRadians(eulerRotation.x());
    float yawRadians = Maths::DegreesToRadians(eulerRotation.y());
    float rollRadians = Maths::DegreesToRadians(eulerRotation.z());
    btQuaternion rotationQuat;
    rotationQuat.setEulerZYX(rollRadians, yawRadians, pitchRadians);
    btTransform transform = floor1->GetPhysicsObject()->GetRigidBody()->getWorldTransform();
    transform.setRotation(rotationQuat);
    floor1->GetPhysicsObject()->GetRigidBody()->setWorldTransform(transform);
    return floor1;
}

GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
    GameObject* sphere = new GameObject();

    // Setting a uniform scale for the sphere
    Vector3 sphereSize = Vector3(radius, radius, radius);

    // Setting the transform properties for the sphere
    sphere->setInitialPosition(position);
    sphere->setRenderScale(sphereSize);

    // Setting the render object for the sphere
    sphere->SetRenderObject(new RenderObject(sphere, resourceManager->getMeshes().get("Sphere.msh"), defaultTexture));
    sphere->SetPhysicsObject(new PhysicsObject(sphere));

    // Creating a Bullet collision shape for the sphere
    btCollisionShape* shape = new btSphereShape(radius);

    // Setting the collision margin for the sphere
    shape->setMargin(0.01f);

    // Initialize Bullet physics for the sphere
    sphere->GetPhysicsObject()->InitBulletPhysics(bulletWorld, shape, inverseMass);

    world->AddGameObject(sphere);

    return sphere;
}

bool TutorialGame::StartMultiplayerGame(bool isHost) {
    server = new Multiplayer::Server(this, isHost);
    server->InitPacketHandlers();
    server->Start();

    if (!isHost) {
        //server->JoinGame("127.0.0.1", 1.0f);
        std::string host = config.get<std::string>("defaultHost");
        server->JoinGame(host.c_str(), 1.0f);
       
    }
    return server->IsConnected();
}


void TutorialGame::Start() {
    instance->SetState(GameState::STARTING);
    instance->LoadWorldFromFile(10);

    // Setup for a multiplayer game.
    if (instance->server) {
        Lobby* lobby = instance->server->GetLobby();

        //for (auto place : lobby->GetUserColors()) {
        //    if (!place.GetUser().has_value()) continue;
        //    User user = place.GetUser().value();
        for (User user:lobby->GetConnectedUsers()) {
            RespawnPoint* respawn = Respawn::GetInstance()->GetRespawn(user.GetUserID() - 1);
            bool mainPlayer = user == *(instance->server->GetUser());
            PlayerObject* player = instance->InitPlayer(respawn->position, respawn->orientation, mainPlayer);
            player->SetWorldID(user.GetUserID());
            player->SetOwner(user);

            btVector4 playerColor = Color::GetPlayerColor(user.GetUserID() - 1);
            player->SetColor(playerColor);

            if (mainPlayer) {
                instance->player = player;
                instance->playerController = std::make_unique<PlayerController>(instance->player, instance->controller, instance->mainCamera, instance->bulletWorld, instance->renderer);
                instance->playerController->setYaw(respawn->yaw);
            }
        }
    }

    // Setup for a single player game.
    else {
        RespawnPoint* playerRespawn = Respawn::GetInstance()->GetRespawn(1);
        instance->player = instance->InitPlayer(playerRespawn->position, playerRespawn->orientation,true);
        instance->player->SetWorldID(0);

        LaserObject* laser = instance->player->GetLaser();
        laser->SetColor(Color::GetPlayerColor(0));

        instance->player->getGun()->GetRenderObject()->SetColour(Color::GetPlayerColor(0));

        instance->playerController = std::make_unique<PlayerController>(instance->player, instance->controller, instance->mainCamera, instance->bulletWorld, instance->renderer);
        instance->spGameController = new SPGameController(instance->player, instance, instance->renderer);
    }
    Shoot::GetInstance()->Initialise(instance->bulletWorld,instance->resourceManager.get(), instance->world.get(), instance->renderer->GetDecalSystem());
    Shoot::GetInstance()->InitShotMasks(instance->player, instance->player->getGun());
}
