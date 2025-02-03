#pragma once

#include <memory>

#include "btBulletDynamicsCommon.h"

class BulletWorld
{
public:
    BulletWorld();

    void step(float dt);
private:
    std::unique_ptr<btDefaultCollisionConfiguration> config;
    std::unique_ptr<btCollisionDispatcher> dispatcher;
    std::unique_ptr<btBroadphaseInterface> overlapCache;
    std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
    std::unique_ptr<btDiscreteDynamicsWorld> world;
};
