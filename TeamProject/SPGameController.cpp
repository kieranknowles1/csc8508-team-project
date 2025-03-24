#include "SPGameController.h"
#include "TutorialGame.h"
#include <algorithm>

using namespace NCL;
using namespace CSC8503;

SPGameController::SPGameController(GameObject* p, TutorialGame* g, GameTechRendererInterface* r)
	: player(p), game(g), renderer(r) {

    btDiscreteDynamicsWorld* bulletWorld = game->getBulletWorld();

    bottom = new NavMesh(bulletWorld);
    bottom->LoadFromFile("bottom.navmesh");
    navMeshes.push_back(bottom);

    top = new NavMesh(bulletWorld);
    top->LoadFromFile("top.navmesh");
    navMeshes.push_back(top);

    front = new NavMesh(bulletWorld);
    front->LoadFromFile("front.navmesh");
    navMeshes.push_back(front);

    back = new NavMesh(bulletWorld);
    back->LoadFromFile("back.navmesh");
    navMeshes.push_back(back);

    left = new NavMesh(bulletWorld);
    left->LoadFromFile("left.navmesh");
    navMeshes.push_back(left);

    right = new NavMesh(bulletWorld);
    right->LoadFromFile("right.navmesh");
    navMeshes.push_back(right);

    for (int i = 101; i <= 150; i++) { laserIDs.push_back(i); }

    score = 0;
    level = 1;
    defeated = 0;

    InitLevel(level);
}

void SPGameController::InitLevel(int curLevel) {
    ClearAIs();

    for (int i = 0; (i < std::max(5 + curLevel, 49)); i++) {
        AddWandererToWorld(bottom, Side::BOTTOM);
        AddWandererToWorld(top, Side::TOP);
        AddWandererToWorld(front, Side::FRONT);
        AddWandererToWorld(back, Side::BACK);
        AddWandererToWorld(left, Side::LEFT);
        AddWandererToWorld(right, Side::RIGHT);
    }
}

void SPGameController::Update(float dt) {
    // Remove deleted wanderers
    wanderers.erase(
        std::remove_if(wanderers.begin(), wanderers.end(),
            [this](const Wanderer* wanderer) {
                if (wanderer->isDeleted()) {
                    defeated++;
                    score += (4 + level) * mult;
                    std::cout << "Score: " << score << std::endl;
                    mult++;
                    multTimer = maxMultTimer;
                    return true;
                }
                return false;
            }),
        wanderers.end()
    );

    multTimer -= dt;
    if (multTimer <= 0) {
        mult = 1;
        multTimer = 0;
    }

    if (defeated > level * 3) {
        level++;
        defeated = 0;
        InitLevel(level);
    }

    // Update remaining wanderers
    for (Wanderer* wanderer : wanderers) {
        wanderer->Update(dt);
    }

    if (navMeshDebug) VisualiseNavMesh();
}

void SPGameController::ClearAIs() {
    while (wanderers.size() > 0) {
        Wanderer* wanderer = wanderers.back();
        AddIDToPool(wanderer->laserID);
        wanderer->DestroyWanderer();
        wanderers.pop_back();
    }
}

Wanderer* SPGameController::AddWandererToWorld(NavMesh* navMesh, Side side) {
    if (laserIDs.size() == 0) {
        //std::cout << "No AI Laser IDs" << std::endl;
        return nullptr;
    }
    Wanderer* wanderer = new Wanderer(player, navMesh, side, GetIDFromPool(), renderer);

    float height = 4.0f;
    float radius = 2.0f;

    wanderer->setInitialPosition(navMesh->GetRandomPointInNavMesh());
    wanderer->setRenderScale(btVector3(radius * 2, height, radius * 2));

    btCollisionShape* shape = new btCapsuleShape(radius, height);

    wanderer->SetRenderObject(new RenderObject(wanderer, game->getResourceManager()->getMeshes().get("Capsule.msh"), game->getDefaultTexture()));

    PhysicsObject* physicsObject = new PhysicsObject(wanderer);
    physicsObject->InitBulletPhysics(game->getBulletWorld(), shape, 0);
    wanderer->SetPhysicsObject(physicsObject);

    wanderer->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

    wanderer->InitPosAndOffset();

    game->getWorld()->AddGameObject(wanderer);

    wanderers.push_back(wanderer);
    return wanderer;
}

Turret* SPGameController::AddTurretToWorld() {
    Turret* turret = new Turret(player);

    Vector3 dimensions = Vector3(5, 5, 5);
    turret->setInitialPosition(btVector3(5, 5, 5));
    turret->setRenderScale(dimensions);

    turret->SetRenderObject(new RenderObject(turret, game->getResourceManager()->getMeshes().get("Kitten.msh"), game->getDefaultTexture()));

    btCollisionShape* shape = new btBoxShape(btVector3(dimensions.x / 2.0f, dimensions.y / 2.0f, dimensions.z / 2.0f));

    shape->setMargin(0.01f);

    PhysicsObject* physicsObject = new PhysicsObject(turret);
    physicsObject->InitBulletPhysics(game->getBulletWorld(), shape, 0);

    turret->SetPhysicsObject(physicsObject);

    turret->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));

    game->getWorld()->AddGameObject(turret);

    testTurret = turret;

    return turret;
}

void SPGameController::VisualiseNavMesh() {
    for (NavMesh* mesh : navMeshes) {
        mesh->VisualiseNavMesh();
    }

    /*btVector3 startPoint(94, 0.5833334, 26);
    btVector3 endPoint(68, 0.5833334, 34);

    btIDebugDraw* debugDrawer = bulletWorld->getDebugDrawer();

    // Draw vertical lines at start and end points
    debugDrawer->drawLine(startPoint, startPoint + btVector3(0, 10, 0), btVector3(0, 1, 0));
    debugDrawer->drawLine(endPoint, endPoint + btVector3(0, 10, 0), btVector3(0, 0, 1));

    // Find path and draw it
    std::vector<btVector3> path = navMesh->FindPath(startPoint, endPoint);
    //navMesh->DebugDrawPath(path);*/
}