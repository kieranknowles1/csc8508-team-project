#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "StateGameObject.h"
#include "BulletDebug.h"

#include <CSC8503CoreClasses/Debug.h>


using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame() : controller(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse()) {
	/* Initializing the Bullet Physics World here as it should be done before Initialize the NCL framework's PhysicsSystem */
	InitBullet();

	world		= new GameWorld();
#ifdef USEVULKAN
	renderer	= new GameTechVulkanRenderer(*world);
	renderer->Init();
	renderer->InitStructures();
#else
	renderer = new GameTechRenderer(*world);
#endif

	world->GetMainCamera().SetController(controller);
	mainCamera = &world->GetMainCamera();
	controller.MapAxis(0, "Sidestep");
	controller.MapAxis(1, "UpDown");
	controller.MapAxis(2, "Forward");

	controller.MapAxis(3, "XLook");
	controller.MapAxis(4, "YLook");

	controller.MapButton(2, "JumpButton");

	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes,
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	planeMesh = renderer->LoadMesh("Plane.msh");
	cubeMesh	= renderer->LoadMesh("cube.msh");
	sphereMesh	= renderer->LoadMesh("sphere.msh");
	catMesh		= renderer->LoadMesh("ORIGAMI_Chat.msh");
	kittenMesh	= renderer->LoadMesh("Kitten.msh");

	enemyMesh	= renderer->LoadMesh("Keeper.msh");
	bonusMesh	= renderer->LoadMesh("19463_Kitten_Head_v1.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");

	//EG Assets:

	//EG character meshes:
	maxMesh = renderer->LoadMesh("/Max/Rig_Maximilian.msh");
	maleguardMesh = renderer->LoadMesh("/MaleGuard/Male_Guard.msh");
	femaleguardMesh = renderer->LoadMesh("/FemaleGuard/Female_Guard.msh");

	basicTex	= renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");
	//Added Shaders:
	orangeShader = renderer->LoadShader("flatvert.glsl", "orangefrag.glsl");
	blueShader = renderer->LoadShader("flatVert.glsl", "bluefrag.glsl");

	InitCamera();
	InitWorld();
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete catMesh;
	delete kittenMesh;
	delete enemyMesh;
	delete bonusMesh;

	delete maxMesh;
	delete maleguardMesh;
	delete femaleguardMesh;

	delete basicTex;
	delete basicShader;
	delete orangeShader;
	delete blueShader;

	delete renderer;
	delete world;
}

static bool BulletRaycast(btDynamicsWorld* world, const btVector3& start, const btVector3& end, btCollisionWorld::ClosestRayResultCallback& resultCallback) {
	world->rayTest(start, end, resultCallback);
	return resultCallback.hasHit();
}

void TutorialGame::UpdateGame(float dt) {
	if (!freeCam) {
		world->GetMainCamera().UpdateCamera(dt, false);
		playerController->UpdateMovement(dt);
	}
	else {
		world->GetMainCamera().UpdateCamera(dt, true);
	}

	UpdateKeys();

	/* Took a while to figure out why gravity wasn't working, I figured out after 
	    reading the doc for the nth time that I forgot to update the world each frame
		with bullet world's step simulation. Otherwise, physics interactions will not
		occur
	*/
	if (bulletWorld->stepSimulation(dt, 10)) {
		FixedUpdate();
	};

	bulletWorld->debugDrawWorld();
	if (testTurret) {
		testTurret->Update(dt);
	}

	bulletWorld->stepSimulation(dt, 10);

	// If the object exists, log its position after the simulation step
	if (objectToTestBulletPhysics) {
		btRigidBody* rigidBody = objectToTestBulletPhysics->GetPhysicsObject()->GetRigidBody();

		if (rigidBody) {
			btVector3 currentPos = rigidBody->getCenterOfMassPosition();
			std::cout << "Current Position of Test Cube: "
				<< currentPos.getX() << ", "
				<< currentPos.getY() << ", "
				<< currentPos.getZ() << std::endl;
		}
	}

	world->UpdateWorld(dt);
	renderer->Update(dt);

	renderer->Render();
	Debug::UpdateRenderables(dt);
}

void TutorialGame::FixedUpdate() {
	// set position to be equal to player
	if (!freeCam) {
		btTransform transformPlayer = player->GetPhysicsObject()->GetRigidBody()->getWorldTransform();
		btVector3 playerPos = transformPlayer.getOrigin();
		playerPos.setY(playerPos.getY() + 3);
		mainCamera->SetPosition(playerPos);
	}
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F3)) {
		bulletDebug->toggle();
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F)) {
		freeCam = !freeCam;
	}
}


/* Bullet Physics world has been initialized here */
void TutorialGame::InitBullet() {
	broadphase = new btDbvtBroadphase();
	collisionConfig = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfig);
	solver = new btSequentialImpulseConstraintSolver();

	bulletWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfig);
	bulletWorld->setGravity(btVector3(0, -9.8, 0));

	bulletDebug = new BulletDebug();
	bulletWorld->setDebugDrawer(bulletDebug);
}

void TutorialGame::InitCamera() {
	world->GetMainCamera().SetNearPlane(0.1f);
	world->GetMainCamera().SetFarPlane(500.0f);
	world->GetMainCamera().SetPitch(-15.0f);
	world->GetMainCamera().SetYaw(315.0f);
	world->GetMainCamera().SetPosition(Vector3(-60, 40, 60));
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	// TODO: Clear bullet
	//physics->Clear();

	InitDefaultFloor();

	// Use this as a reference to create more cube objects
	AddCubeToWorld(Vector3(0, 30, 0), Vector3(5, 5, 5), 1.0f);

	// Use this as a reference to create more sphere objects
	AddSphereToWorld(Vector3(10, 30, 0), 5.0f, 1.0f);

	// Use this as a reference to create more capsule objects
	AddCapsuleToWorld(Vector3(20, 15, 0), 4.0f, 2.0f, 1.0f);
	InitPlayer();

}

void TutorialGame::InitPlayer() {

	player = AddCapsuleToWorld(Vector3(10, 4, 20), 4.0f, 2.0f, 10.0f);
	player->GetPhysicsObject()->GetRigidBody()->setAngularFactor(0);
	player->GetPhysicsObject()->GetRigidBody()->setFriction(1);
	player->GetPhysicsObject()->GetRigidBody()->setDamping(0.999, 0);
	playerController = new PlayerController(player, controller, mainCamera, bulletWorld);

	AddTurretToWorld();
}

Turret* TutorialGame::AddTurretToWorld() {
	Turret* turret = new Turret();

	Vector3 dimensions = Vector3(5, 5, 5);
	turret->setInitialPosition(btVector3(5, 5, 5));
	turret->setRenderScale(dimensions);

	turret->SetRenderObject(new RenderObject(turret, kittenMesh, basicTex, basicShader));

	btCollisionShape* shape = new btBoxShape(btVector3(dimensions.x / 2.0f, dimensions.y / 2.0f, dimensions.z / 2.0f));

	shape->setMargin(0.01f);

	PhysicsObject* physicsObject = new PhysicsObject(turret);
	physicsObject->InitBulletPhysics(bulletWorld, shape, 1.0f);

	turret->SetPhysicsObject(physicsObject);

	turret->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));

	world->AddGameObject(turret);

	testTurret = turret;

	return turret;
}

/* Adding an object to test the bullet physics */
GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
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
	cube->GetPhysicsObject()->InitBulletPhysics(bulletWorld, shape, inverseMass);

	// Setting render object
	cube->SetRenderObject(new RenderObject(cube, cubeMesh, basicTex, basicShader));

	world->AddGameObject(cube);

	return cube;
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
	capsule->SetRenderObject(new RenderObject(capsule, capsuleMesh, basicTex, basicShader));

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
	plane->SetRenderObject(new RenderObject(plane, planeMesh, basicTex, basicShader));

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
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();

	Vector3 floorSize = Vector3(200, 2, 200);

	// Setting the transform properties for the floor
	floor->setInitialPosition(position);
	floor->setRenderScale(floorSize);

	// Creating a bullet collision shape
	btCollisionShape* shape = new btBoxShape(btVector3(floorSize.x / 2.0f, floorSize.y / 2.0f, floorSize.z / 2.0f));

	// Setting the render object for the floor
	floor->SetRenderObject(new RenderObject(floor, cubeMesh, basicTex, basicShader));

	// Setting the physics object for the floor
	floor->SetPhysicsObject(new PhysicsObject(floor));

	// Setting the collision margin for the floor
	shape->setMargin(0.01f);

	// Initializing the physics object for the floor
	floor->GetPhysicsObject()->InitBulletPhysics(bulletWorld, shape, 0.0f);

	// Adding the floor to the game world
	world->AddGameObject(floor);

	return floor;
}

GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject();

	// Setting a uniform scale for the sphere
	Vector3 sphereSize = Vector3(radius, radius, radius);

	// Setting the transform properties for the sphere
	sphere->setInitialPosition(position);
	sphere->setRenderScale(sphereSize);

	// Setting the render object for the sphere
	sphere->SetRenderObject(new RenderObject(sphere, sphereMesh, basicTex, basicShader));
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

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -20, 0));
}

