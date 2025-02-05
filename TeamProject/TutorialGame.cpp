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

	forceMagnitude	= 10.0f;
	useGravity		= false;
	inSelectionMode = false;

	world->GetMainCamera().SetController(controller);

	controller.MapAxis(0, "Sidestep");
	controller.MapAxis(1, "UpDown");
	controller.MapAxis(2, "Forward");

	controller.MapAxis(3, "XLook");
	controller.MapAxis(4, "YLook");

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

	basicTex	= renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");

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

	delete basicTex;
	delete basicShader;

	delete renderer;
	delete world;
}

void TutorialGame::UpdateGame(float dt) {
	if (!inSelectionMode) {
		world->GetMainCamera().UpdateCamera(dt);
	}
	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix::View(camPos, objPos, Vector3(0,1,0));

		Matrix4 modelMat = Matrix::Inverse(temp);

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera().SetPosition(camPos);
		world->GetMainCamera().SetPitch(angles.x);
		world->GetMainCamera().SetYaw(angles.y);
	}

	UpdateKeys();

	/*if (useGravity) {
		Debug::Print("(G)ravity on", Vector2(5, 95), Debug::RED);
	}
	else {
		Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED);
	}*/
	//This year we can draw debug textures as well!
	//Debug::DrawTex(*basicTex, Vector2(10, 10), Vector2(5, 5), Debug::MAGENTA);

	RayCollision closestCollision;
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::K) && selectionObject) {
		Vector3 rayPos;
		Vector3 rayDir;

		rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);

		rayPos = selectionObject->GetTransform().GetPosition();

		Ray r = Ray(rayPos, rayDir);

		if (world->Raycast(r, closestCollision, true, selectionObject)) {
			if (objClosest) {
				objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
			}
			objClosest = (GameObject*)closestCollision.node;

			objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
		}
	}

	Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 0, 1));

	SelectObject();
	MoveSelectedObject();

	/* Took a while to figure out why gravity wasn't working, I figured out after 
	    reading the doc for the nth time that I forgot to update the world each frame
		with bullet world's step simulation. Otherwise, physics interactions will not
		occur 
	*/

	bulletWorld->stepSimulation(dt, 10);
	bulletWorld->debugDrawWorld();

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

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F3)) {
		bulletDebug->toggle();
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::G)) {
		// Bullet physics gravity will be on all the time, no toggling
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera().BuildViewMatrix();
	Matrix4 camWorld	= Matrix::Inverse(view);

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis = Vector::Normalise(fwdAxis);

	if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
		selectionObject->GetPhysicsObject()->AddForce(fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
		selectionObject->GetPhysicsObject()->AddForce(-fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::NEXT)) {
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0,-10,0));
	}
}

void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyCodes::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
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
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	// TODO: Clear bullet
	//physics->Clear();

	//InitMixedGridWorld(15, 15, 3.5f, 3.5f);

	//InitGameExamples();
	InitDefaultFloor();

	// Use this as a reference to create more cube objects
	AddCubeToWorld(Vector3(0, 30, 0), Vector3(5, 5, 5), 1.0f);

	// Use this as a reference to create more sphere objects
	AddSphereToWorld(Vector3(10, 30, 0), 5.0f, 1.0f);

	// Use this as a reference to create more capsule objects
	AddCapsuleToWorld(Vector3(20, 30, 0), 3.0f, 2.0f, 1.0f);

}

/* Adding an object to test the bullet physics */
GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	// Setting the transform properties for the cube
	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions);

	// Setting render object
	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));

	// Setting the physics object for the cube
	cube->SetPhysicsObject(new PhysicsObject(cube));

	// Creating Bullet collision shape
	btCollisionShape* shape = new btBoxShape(btVector3(dimensions.x / 2.0f, dimensions.y / 2.0f, dimensions.z / 2.0f));

	// The object is penetrating the floor a bit, so I reduced the bullet collison margin to avoid sinking in the floor
	shape->setMargin(0.01f);

	// Initialize Bullet physics for the cube
	cube->GetPhysicsObject()->InitBulletPhysics(bulletWorld, shape, inverseMass);

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddCapsuleToWorld(const Vector3& position, float height, float radius, float inverseMass) {
	GameObject* capsule = new GameObject();
	
	// Setting the transform properties for the capsule
	capsule->GetTransform()
		.SetPosition(position)
		.SetScale(Vector3(radius * 2, height, radius * 2)); // Radius needs to be scaled to match the top and bottom caps of the capsule

	// Setting the render object for the capsule
	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, basicShader));

	// Setting the physics object for the capsule
	capsule->SetPhysicsObject(new PhysicsObject(capsule));

	// Creating a Bullet collision shape for the capsule
	btCollisionShape* shape = new btCapsuleShape(radius, height);

	// Initializing the physics object for the capsule
	capsule->GetPhysicsObject()->InitBulletPhysics(bulletWorld, shape, inverseMass);

	world->AddGameObject(capsule);

	return capsule;
}

GameObject* TutorialGame::AddInfinitePlaneToWorld(const Vector3& position, const Vector3& normal, float planeConstant) {
	GameObject* plane = new GameObject();

	// Set the transform properties for the plane
	plane->GetTransform()
		.SetPosition(position)
		.SetScale(Vector3(1, 1, 1)); // Scale does not affect Bullet's infinite plane

	// Create Bullet collision shape for an infinite plane
	btCollisionShape* shape = new btStaticPlaneShape(btVector3(normal.x, normal.y, normal.z), planeConstant);

	// Set the render object
	plane->SetRenderObject(new RenderObject(&plane->GetTransform(), planeMesh, basicTex, basicShader));

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
	floor->GetTransform()
		.SetPosition(position)
		.SetScale(floorSize);

	// Setting the render object for the floor
	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));

	// Setting the physics object for the floor
	floor->SetPhysicsObject(new PhysicsObject(floor));

	// Creating a bullet collision shape
	btCollisionShape* shape = new btBoxShape(btVector3(floorSize.x / 2.0f, floorSize.y / 2.0f, floorSize.z/ 2.0f));

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
	sphere->GetTransform()
		.SetPosition(position)
		.SetScale(sphereSize);

	// Setting the render object for the sphere
	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
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

// TODO: From 8503. Probably donnt need
//GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
//	float meshSize		= 1.0f;
//	float inverseMass	= 0.5f;
//
//	GameObject* character = new GameObject();
//	SphereVolume* volume  = new SphereVolume(1.0f);
//
//	character->SetBoundingVolume((CollisionVolume*)volume);
//
//	character->GetTransform()
//		.SetScale(Vector3(meshSize, meshSize, meshSize))
//		.SetPosition(position);
//
//	character->SetRenderObject(new RenderObject(&character->GetTransform(), catMesh, nullptr, basicShader));
//	character->SetPhysicsObject(new PhysicsObject(character));
//
//	character->GetPhysicsObject()->SetInverseMass(inverseMass);
//	character->GetPhysicsObject()->InitSphereInertia();
//
//	world->AddGameObject(character);
//
//	return character;
//}
//
//GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
//	float meshSize		= 3.0f;
//	float inverseMass	= 0.5f;
//
//	GameObject* character = new GameObject();
//
//	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
//	character->SetBoundingVolume((CollisionVolume*)volume);
//
//	character->GetTransform()
//		.SetScale(Vector3(meshSize, meshSize, meshSize))
//		.SetPosition(position);
//
//	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
//	character->SetPhysicsObject(new PhysicsObject(character));
//
//	character->GetPhysicsObject()->SetInverseMass(inverseMass);
//	character->GetPhysicsObject()->InitSphereInertia();
//
//	world->AddGameObject(character);
//
//	return character;
//}
//
//GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
//	GameObject* apple = new GameObject();
//
//	SphereVolume* volume = new SphereVolume(0.5f);
//	apple->SetBoundingVolume((CollisionVolume*)volume);
//	apple->GetTransform()
//		.SetScale(Vector3(2, 2, 2))
//		.SetPosition(position);
//
//	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
//	apple->SetPhysicsObject(new PhysicsObject(apple));
//
//	apple->GetPhysicsObject()->SetInverseMass(1.0f);
//	apple->GetPhysicsObject()->InitSphereInertia();
//
//	world->AddGameObject(apple);
//
//	return apple;
//}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -20, 0));
}

void TutorialGame::InitGameExamples() {
	//AddPlayerToWorld(Vector3(0, 5, 0));
	//AddEnemyToWorld(Vector3(5, 5, 0));
	//AddBonusToWorld(Vector3(10, 5, 0));
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

/*
Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		Debug::Print("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Left)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			// TODO: Do the bullet way
			//Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

			//RayCollision closestCollision;
			//if (world->Raycast(ray, closestCollision, true)) {
			//	selectionObject = (GameObject*)closestCollision.node;

			//	selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
			//	return true;
			//}
			//else {
			//	return false;
			//}
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}
	}
	else {
		Debug::Print("Press Q to change to select mode!", Vector2(5, 85));
	}
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) {
		return;//we haven't selected anything!
	}
	//Push the selected object!
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::Right)) {
		// TODO: Bullet
		//Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

		//RayCollision closestCollision;
		//if (world->Raycast(ray, closestCollision, true)) {
		//	if (closestCollision.node == selectionObject) {
		//		selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
		//	}
		//}
	}
}


