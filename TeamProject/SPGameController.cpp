#include "SPGameController.h"
#include "TutorialGame.h"
#include "PlayerObject.h"
#include <algorithm>
#include <CSC8503CoreClasses/Debug.h>

using namespace NCL;
using namespace CSC8503;

SPGameController::SPGameController(PlayerObject* p, TutorialGame* g, GameTechRendererInterface* r)
	: player(p), game(g), renderer(r) {

    if (navMeshDebug) g->SetFreeCam(true);

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

    //for (int i = 101; i <= 150; i++) { laserIDs.push_back(i); }

    score = 0;
    level = 1;
    defeated = 0;

    InitLevel(level);
}

void SPGameController::InitLevel(int curLevel) {
    ClearAIs();

    for (int i = 0; (i < std::min(5 + curLevel, 100)); i++) {
        AddWandererToWorld(bottom, Side::BOTTOM);
        AddWandererToWorld(top, Side::TOP);
        AddWandererToWorld(front, Side::FRONT);
        AddWandererToWorld(back, Side::BACK);
        //AddWandererToWorld(left, Side::LEFT);
        //AddWandererToWorld(right, Side::RIGHT);
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

    int totalWanderers = std::min(5 + level, 100) * 6;

    // 0.33 ~ 2 walls worth
    if (defeated > std::floor(totalWanderers * 0.33f)) {
        level++;
        defeated = 0;
        InitLevel(level);
    }

    // Update remaining wanderers
    for (Wanderer* wanderer : wanderers) {
        wanderer->Update(dt);
    }

    std::string out = "Score: " + std::to_string(score);
    Debug::Print(out, Vector2(0.05f, 0.05f));

    out = "Mult: " + std::to_string(mult);
    Debug::Print(out, Vector2(0.05f, 0.1f));

    out = "Level: " + std::to_string(level);
    Debug::Print(out, Vector2(0.05f, 0.15f));

    if (navMeshDebug) VisualiseNavMesh();
}

void SPGameController::ClearAIs() {
    for (Wanderer* wanderer : wanderers) {
        if (wanderer) wanderer->DestroyWanderer();
    }
    wanderers.clear();
}

Wanderer* SPGameController::AddWandererToWorld(NavMesh* navMesh, Side side) {
    Wanderer* wanderer = new Wanderer(player, navMesh, side, renderer, level);

    float height = 16.0f;
    float radius = 8.0f;

    wanderer->setInitialPosition(navMesh->GetRandomPointInNavMesh());
    wanderer->setRenderScale(btVector3(radius * 2, height, radius * 2));

    btCollisionShape* shape = new btCapsuleShape(radius, height);

    wanderer->SetRenderObject(new RenderObject(wanderer, game->getResourceManager()->getMeshes().get("Capsule.msh"), game->getDefaultTexture()));

    PhysicsObject* physicsObject = new PhysicsObject(wanderer);
    physicsObject->InitBulletPhysics(game->getBulletWorld(), shape, 0);
    wanderer->SetPhysicsObject(physicsObject);

    wanderer->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

    wanderer->InitPosAndOffset();

    LaserObject* laser = new LaserObject(wanderer);
    laser->SetColor(Color::GetPlayerColor(0));
    laser->SetThickness(0.1f);

    renderer->TrackLaser(laser);
    wanderer->SetLaser(laser);

    game->GetWorld()->AddGameObject(wanderer);
    game->GetWorld()->AddGameObject(laser);

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

    game->GetWorld()->AddGameObject(turret);

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