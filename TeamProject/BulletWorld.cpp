#include "BulletWorld.h"

BulletWorld::BulletWorld() {
    config = std::make_unique<btDefaultCollisionConfiguration>();
    dispatcher = std::make_unique<btCollisionDispatcher>(config.get());
    overlapCache = std::make_unique<btDbvtBroadphase>();
    solver = std::make_unique<btSequentialImpulseConstraintSolver>();
    world = std::make_unique<btDiscreteDynamicsWorld>(
        dispatcher.get(), overlapCache.get(), solver.get(), config.get()
    );
    world->setGravity(btVector3(0, -10, 0));

    for (int i = 0; i < 1; i++) {
        btCollisionShape* shape = new btBoxShape(btVector3(1, 1, 1));
        btTransform trans;
        trans.setIdentity();
        trans.setOrigin(btVector3(2, 10, 0));

        btScalar mass = 1.0f;
        btVector3 inertia(0, 0, 0);
        shape->calculateLocalInertia(mass, inertia);

        btDefaultMotionState* state = new btDefaultMotionState(trans);
        btRigidBody::btRigidBodyConstructionInfo info(
            mass, state, shape, inertia
        );
        btRigidBody* body = new btRigidBody(info);
        world->addRigidBody(body);
    }
}

void BulletWorld::step(float dt) {
    world->stepSimulation(dt);

    for (int i = 0; i < world->getNumCollisionObjects(); i++) {
        auto obj = world->getCollisionObjectArray()[i];
        auto body = btRigidBody::upcast(obj);
        btTransform trans;
        if (body && body->getMotionState()) {
            body->getMotionState()->getWorldTransform(trans);
        } else {
            trans = obj->getWorldTransform();
        }
        printf(" world pos object %d = %f ,%f ,%f \n ",
            i,
            float(trans.getOrigin().getX()),
            float(trans.getOrigin().getY()),
            float(trans.getOrigin().getZ())
        );
    }
}
