#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "StateGameObject.h"
#include "BulletDebug.h"

#include <CSC8503CoreClasses/Debug.h>
#include <NCLCoreClasses/Window.h>

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame(Render::GameRenderer* renderer)
	: controller(*Window::GetWindow()->GetKeyboard()
	, *Window::GetWindow()->GetMouse())
{
	this->renderer = renderer;
	/* Initializing the Bullet Physics World here as it should be done before Initialize the NCL framework's PhysicsSystem */
	//InitBullet(); //bullet is initialised in initialiseAssets already

	world		= new GameWorld();

	world->GetMainCamera().SetController(controller);
	mainCamera = &world->GetMainCamera();

	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes,
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	auto backend = renderer->getBackend();
	planeMesh = backend->loadMesh("Plane.msh");
	cubeMesh = backend->loadMesh("Cube.msh");
	sphereMesh = backend->loadMesh("Sphere.msh");
	catMesh = backend->loadMesh("ORIGAMI_Chat.msh");
	kittenMesh = backend->loadMesh("Kitten.msh");

	enemyMesh = backend->loadMesh("Keeper.msh");
	bonusMesh = backend->loadMesh("19463_Kitten_Head_v1.msh");
	capsuleMesh = backend->loadMesh("Capsule.msh");

	//EG Assets:

	//EG character meshes:
	maxMesh = backend->loadMesh("/Max/Rig_Maximilian.msh");
	maleguardMesh = backend->loadMesh("/MaleGuard/Male_Guard.msh");
	femaleguardMesh = backend->loadMesh("/FemaleGuard/Female_Guard.msh");

	basicTex	= backend->loadTexture("checkerboard.png");
	basicShader = backend->loadShader("scene.vert", "scene.frag");
	//Added Shaders:
	flatShader = backend->loadShader("flatvert.glsl", "flatfrag.glsl");

	InitCamera();
	InitWorld();
}

TutorialGame::~TutorialGame()	{
	DestroyBullet();
	// TODO: Should we use a proper resource manager or smart pointers?
	delete planeMesh;
	delete cubeMesh;
	delete sphereMesh;
	delete catMesh;
	delete kittenMesh;
	delete enemyMesh;
	delete bonusMesh;
	delete capsuleMesh;

	delete maxMesh;
	delete maleguardMesh;
	delete femaleguardMesh;

	delete basicTex;
	delete basicShader;
	delete flatShader;

	delete renderer;
	delete world;

	delete playerController;
}

static bool BulletRaycast(btDynamicsWorld* world, const btVector3& start, const btVector3& end, btCollisionWorld::ClosestRayResultCallback& resultCallback) {
	world->rayTest(start, end, resultCallback);
	return resultCallback.hasHit();
}

void TutorialGame::UpdateGame(float dt) {
	// Old
	//int substeps = std::floor(dt / PHYSICS_PERIOD);
	//int steps = bulletWorld->stepSimulation(dt , substeps, PHYSICS_PERIOD);

	////New
	int substeps = 0;
	float maxDt = btMin(PHYSICS_PERIOD, dt);
	int steps = bulletWorld->stepSimulation(maxDt, substeps, PHYSICS_PERIOD);

	bulletWorld->debugDrawWorld();
	if (testTurret) {
		testTurret->Update(dt);
	}
	UpdateKeys();
	world->UpdateWorld(dt);
	world->OperateOnContents([&](GameObject* obj) {
		obj->GetPhysicsObject()->CheckCollisions(bulletWorld);
	});

	// Press F for freeCam, press G for thirdPerson
	if (freeCam) {
		//freeCam Movement
		world->GetMainCamera().UpdateCamera(dt, true);
	}
	else {
		//player Movement
		world->GetMainCamera().UpdateCamera(dt, false);
		playerController->UpdateMovement(dt);
		if (thirdPerson) {
			ThirdPersonControls();
		}
	}
	//player invisable in first person
	if (!thirdPerson && !freeCam) {
		Vector4 colour = player->GetRenderObject()->GetColour();
		colour.w = 0;
		player->GetRenderObject()->SetColour(colour);
	}
	else {
		Vector4 colour = player->GetRenderObject()->GetColour();
		colour.w = 1;
		player->GetRenderObject()->SetColour(colour);
	}

	renderer->drawWorld(world);

	Debug::UpdateRenderables(dt);
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
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::G)) {
		thirdPerson = !thirdPerson;
		playerController->SetThirdPerson(thirdPerson);
	}
}

void TutorialGame::ThirdPersonControls() {
	btTransform transformPlayer = player->GetPhysicsObject()->GetRigidBody()->getWorldTransform();
	btQuaternion playerRotation = transformPlayer.getRotation();

	// Extract yaw from playerRotation
	btMatrix3x3 rotationMatrix(playerRotation);
	btVector3 forward = rotationMatrix * btVector3(0, 0, -1);

	// Zero out the Y component to ignore pitch/roll
	forward.setY(0);
	forward.normalize(); // Ensure it's a unit vector

	// Camera offset (up 10, back 30)
	btVector3 cameraOffset(0, 10, 30);
	btVector3 cameraPosition = transformPlayer.getOrigin() - (forward * cameraOffset.z()) + btVector3(0, cameraOffset.y(), 0);

	mainCamera->SetPosition(cameraPosition);

	// Compute yaw correctly from forward vector
	float playerYaw = Maths::RadiansToDegrees(atan2(forward.x(), forward.z())) + 180.0f;
	mainCamera->SetYaw(playerYaw);
	mainCamera->SetPitch(-15);

}

void TutorialGame::DestroyBullet() {
	world->OperateOnContents([&](GameObject* obj) {
		if (obj->GetPhysicsObject()) {
			obj->GetPhysicsObject()->removeFromBullet(bulletWorld);
		}
	});

	delete bulletWorld;
	delete bulletDebug;
	delete solver;
	delete dispatcher;
	delete collisionConfig;
	delete broadphase;
}

/* Bullet Physics world has been initialized here */
void TutorialGame::InitBullet() {
	broadphase = new btDbvtBroadphase();
	collisionConfig = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfig);
	solver = new btSequentialImpulseConstraintSolver();

	bulletWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfig);
	bulletWorld->setGravity(btVector3(0, -30.0f, 0));
	bulletDebug = new BulletDebug();
	bulletWorld->setDebugDrawer(bulletDebug);
}

void TutorialGame::InitCamera() {
	mainCamera->SetFieldOfVision(90);
	world->GetMainCamera().SetNearPlane(0.1f);
	world->GetMainCamera().SetFarPlane(5000.0f);
	world->GetMainCamera().SetPitch(-15.0f);
	world->GetMainCamera().SetYaw(315.0f);
	world->GetMainCamera().SetPosition(Vector3(-60, 40, 60));
}

void TutorialGame::InitWorld() {
	DestroyBullet();
	world->ClearAndErase();
	InitBullet();

	//floors
	AddFloorToWorld(Vector3(0, 0, 0), Vector3(500, 2, 500), Vector3(0, 0, 0), true)->SetName("Floor1");
	AddFloorToWorld(Vector3(498.5, -21.75, 0), Vector3(500, 2, 500), Vector3(0, 0, -5),true)->SetName("Floor2");
	AddFloorToWorld(Vector3(996, -43.6, 0), Vector3(500, 2, 500), Vector3(0, 0, 0),true)->SetName("Floor3");

	//walls
	AddFloorToWorld(Vector3(1245, 206, 0), Vector3(2, 500, 500), Vector3(0, 0, 0),false);
	AddFloorToWorld(Vector3(996, 206, 249), Vector3(500, 500, 2), Vector3(0, 0, 0), false);
	AddFloorToWorld(Vector3(996, 206, -249), Vector3(500, 500, 2), Vector3(0, 0, 0), false);
	AddFloorToWorld(Vector3(498, 206, 249), Vector3(500, 500, 2), Vector3(0, 0, 0), false);
	AddFloorToWorld(Vector3(498, 206, -249), Vector3(500, 500, 2), Vector3(0, 0, 0), false);
	AddFloorToWorld(Vector3(-2, 206, 249), Vector3(500, 500, 2), Vector3(0, 0, 0), false);
	AddFloorToWorld(Vector3(-2, 206, -249), Vector3(500, 500, 2), Vector3(0, 0, 0), false);
	AddFloorToWorld(Vector3(-248, 206, 0), Vector3(2, 500, 500), Vector3(0, 0, 0), false);

	// Use this as a reference to create more cube objects
	AddCubeToWorld(Vector3(0, 30, 0), Vector3(5, 5, 5), 1.0f);
	AddCubeToWorld(Vector3(500, 30, 0), Vector3(10, 10, 10), 5.0f);
	AddCubeToWorld(Vector3(20, 30, 50), Vector3(7, 7, 7), 4.0f);
	AddCubeToWorld(Vector3(120, 30, -20), Vector3(5, 5, 5), 1.0f);

	AddCubeToWorld(Vector3(100, 8, 100), Vector3(30, 2,30), 0.0f);

	// Use this as a reference to create more sphere objects
	AddSphereToWorld(Vector3(10, 30, 0), 5.0f, 4.0f);
	AddSphereToWorld(Vector3(-30, 30, 0), 7.0f, 8.0f);
	AddSphereToWorld(Vector3(300, 30, -40), 9.0f, 16.0f);

	// Use this as a reference to create more capsule objects
	AddCapsuleToWorld(Vector3(20, 15, 0), 4.0f, 2.0f, 2.0f);
	AddCapsuleToWorld(Vector3(70, 15, -20), 8.0f, 4.0f, 4.0f);
	AddCapsuleToWorld(Vector3(-20, 15, 12), 6.0f, 5.0f, 8.0f);

	InitPlayer();
	AddTurretToWorld();
}

void TutorialGame::InitPlayer() {

	player = AddPlayerCapsuleToWorld(Vector3(10, 5, 20), 4.0f, 2.0f, 10.0f);

	player->GetPhysicsObject()->GetRigidBody()->setAngularFactor(0);
	player->GetPhysicsObject()->GetRigidBody()->setFriction(0.0f);
	player->GetPhysicsObject()->GetRigidBody()->setDamping(0.0, 0);
	gun = AddCubeToWorld(Vector3(10, 2, 20), Vector3(0.6, 0.6, 1.6), 0, false);
	playerController = new PlayerController(player, gun, controller, mainCamera, bulletWorld,world);
	player->GetRenderObject()->SetColour(playerColour);

}

Turret* TutorialGame::AddTurretToWorld() {
	Turret* turret = new Turret(player);

	Vector3 dimensions = Vector3(5, 5, 5);
	turret->setInitialPosition(btVector3(5, 5, 5));
	turret->setRenderScale(dimensions);

	turret->SetRenderObject(new RenderObject(turret, kittenMesh, basicTex, basicShader));

	btCollisionShape* shape = new btBoxShape(btVector3(dimensions.x / 2.0f, dimensions.y / 2.0f, dimensions.z / 2.0f));

	shape->setMargin(0.01f);

	PhysicsObject* physicsObject = new PhysicsObject(turret);
	physicsObject->InitBulletPhysics(bulletWorld, shape, 0);

	turret->SetPhysicsObject(physicsObject);

	turret->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));

	world->AddGameObject(turret);

	testTurret = turret;

	return turret;
}

/* Adding an object to test the bullet physics */
GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass,bool hasCollision) {
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
	cube->SetRenderObject(new RenderObject(cube, cubeMesh, basicTex, basicShader));

	world->AddGameObject(cube);

	return cube;
}

PlayerObject* TutorialGame::AddPlayerCapsuleToWorld(const Vector3& position, float height, float radius, float inverseMass) {
	PlayerObject* capsule = new PlayerObject();

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
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position, const Vector3& size, const Vector3& rotation, bool isFloor) {
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
	floor1->setIsFloor(isFloor);
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
