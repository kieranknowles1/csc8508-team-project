#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"
#include "AudioEngine.h"

#include "BulletDebug.h"

#include <CSC8503CoreClasses/Debug.h>

#include "Window.h"

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame(GameTechRendererInterface* renderer, GameWorld* world, Controller* controller)
	: renderer(renderer)
	, controller(controller)
	, world(world)
{
	/* Initializing the Bullet Physics World here as it should be done before Initialize the NCL framework's PhysicsSystem */
	//InitBullet(); //bullet is initialised in initialiseAssets already
	world->GetMainCamera().SetController(controller);
	mainCamera = &world->GetMainCamera();

	loadFromLevel = true;
	resourceManager = std::make_unique<ResourceManager>(renderer);
	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes,
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	defaultTexture = resourceManager->getTextures().get("checkerboard.png");
	defaultShader = resourceManager->getShaders().get(Shader::Default);

	InitCamera();
	InitWorld();
}

TutorialGame::~TutorialGame()	{
	DestroyBullet();
	audioEngine.Shutdown();

	delete playerController;
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
	float maxDt = btMin(PHYSICS_PERIOD, dt);
	int steps = bulletWorld->stepSimulation(maxDt, substeps, PHYSICS_PERIOD);

	profiler.startSection("Update World");
	if (testTurret) {
		testTurret->Update(dt);
	}

	UpdateKeys();
	world->UpdateWorld(dt);
	profiler.startSection("Check Collisions");
	// Check for collisions
	CheckCollisions();
	
	UpdatePlayer(dt);

	profiler.startSection("Update Camera");


	profiler.startSection("Prepare Render");
	bulletWorld->debugDrawWorld();

	profiler.endFrame();
	if (showProfiling) {
		profiler.printTimes();
	}
}

void TutorialGame::UpdatePlayer(float dt) {

	playerController->CalculateDirections(dt);
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
		// TODO: Proper invisibility
		player->setRenderScale(Vector3(0, 0, 0));

		Vector4 colour = player->GetRenderObject()->GetColour();
		colour.w = 0;
		player->GetRenderObject()->SetColour(colour);
	}
	else {
		Vector4 colour = player->GetRenderObject()->GetColour();
		colour.w = 1;
		player->GetRenderObject()->SetColour(colour);
	}

	/*if (rotateTimer < rotateTime) {
		rotateTimer += dt;
		playerController->setWorldRotation(std::lerp(oldRotate, targetRotate, rotateTimer / rotateTime));
	}
	else if (!finished) {
		playerController->setWorldRotation(targetRotate);
		finished = true;
	}*/

	bulletWorld->setGravity(playerController->getUpDirection() * -30.0f);
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
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F4)) {
		showProfiling = !showProfiling;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F)) {
		freeCam = !freeCam;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::G)) {
		thirdPerson = !thirdPerson;
		playerController->SetThirdPerson(thirdPerson);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q)) {
		btVector3 oldRotate = playerController->getUpDirection();
		btVector3 targetRotate = playerController->CalculateRightDirection(oldRotate);
		playerController->setTargetWorldRotation(targetRotate);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::E)) {
		btVector3 oldRotate = playerController->getUpDirection();
		btVector3 targetRotate = playerController->CalculateRightDirection(oldRotate);
		playerController->setTargetWorldRotation(-targetRotate);
	}
}

void TutorialGame::ThirdPersonControls() {
	btTransform transformPlayer = player->GetPhysicsObject()->GetRigidBody()->getWorldTransform();
	btVector3 up = playerController->getUpDirection();
	btVector3 right = playerController->getRightDirection();
	btVector3 forw = playerController->getForwardDirection();
	btQuaternion playerRotation = transformPlayer.getRotation();
	btMatrix3x3 rotationMatrix(playerRotation);
	btVector3 r = rotationMatrix * btVector3(1, 0, 0);
	btVector3 forward = rotationMatrix * btVector3(0, 0, -1);
	forward = (forward * right.absolute()) + (forward * forw.absolute());
	forward.normalize();
	float camHeight = 10.0f;
	float camDist = 30.0f;
	btVector3 cameraOffset = (-forward * camDist) + (up * camHeight);

	btVector3 cameraPosition = transformPlayer.getOrigin() + cameraOffset;
	mainCamera->SetPosition(cameraPosition);

	float forwardProjX = forw.dot(forward);
	float rightProjX = right.dot(forward);
	float playerYaw = Maths::RadiansToDegrees(atan2(rightProjX, forwardProjX))+180;

	mainCamera->SetYaw(playerYaw);
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
	bulletWorld->setGravity(btVector3(00.0f, -30.0f, 0));
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
	audioEngine.Init();



	if (loadFromLevel) {
		levelImporter = new LevelImporter(resourceManager.get(), world, bulletWorld);
		levelImporter->LoadLevel(6);
		InitPlayer();
		return;
	}

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

	AddTurretToWorld();
	InitPlayer();

}

void TutorialGame::InitPlayer() {
	if (loadFromLevel) {
		player = AddPlayerCapsuleToWorld(Vector3(0, 100, 30), 4.0f, 2.0f, 10.0f);
	}else {
		player = AddPlayerCapsuleToWorld(Vector3(10, 5, 20), 4.0f, 2.0f, 10.0f);
	}
	player->GetPhysicsObject()->GetRigidBody()->setAngularFactor(0);
	player->GetPhysicsObject()->GetRigidBody()->setFriction(0.0f);
	player->GetPhysicsObject()->GetRigidBody()->setDamping(0.0, 0);
	gun = AddCubeToWorld(Vector3(10, 2, 20), Vector3(0.6, 0.6, 1.6), 0, false); 
	playerController = new PlayerController(player, gun, controller, mainCamera, bulletWorld, world, resourceManager.get());
	player->GetRenderObject()->SetColour(playerColour);

}

Turret* TutorialGame::AddTurretToWorld() {
	Turret* turret = new Turret(player);

	Vector3 dimensions = Vector3(5, 5, 5);
	turret->setInitialPosition(btVector3(5, 5, 5));
	turret->setRenderScale(dimensions);

	turret->SetRenderObject(new RenderObject(turret, resourceManager->getMeshes().get("Kitten.msh"), defaultTexture, defaultShader));

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
	cube->SetRenderObject(new RenderObject(cube, resourceManager->getMeshes().get("Cube.msh"), defaultTexture, defaultShader));
	cube->GetRenderObject()->SetTexRepeating(true); //scale texture (no stretching)

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
	capsule->SetRenderObject(new RenderObject(capsule, resourceManager->getMeshes().get("Capsule.msh"), defaultTexture, defaultShader));
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
	capsule->SetRenderObject(new RenderObject(capsule, resourceManager->getMeshes().get("Capsule.msh"), defaultTexture, defaultShader));
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
	plane->SetRenderObject(new RenderObject(plane, resourceManager->getMeshes().get("Plane.msh"), defaultTexture, defaultShader));
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
	sphere->SetRenderObject(new RenderObject(sphere, resourceManager->getMeshes().get("Sphere.msh"), defaultTexture, defaultShader));
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
