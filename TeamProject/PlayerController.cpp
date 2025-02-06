#include "PlayerController.h"


using namespace NCL;
using namespace CSC8503;

void PlayerController::UpdateMovement(float dt) {
    float camRotation = camera->GetYaw(); // Get yaw in degrees
    float yawRad = Maths::DegreesToRadians(camRotation); // Convert to radians


    bool diag = (controller->GetNamedAxis("Sidestep") != 0 && controller->GetNamedAxis("Forward") != 0);
    float moveScale = diag ? 0.5f : 1.0f;

    Vector3 forward = Vector3(sin(yawRad), 0.0f, cos(yawRad));
    Vector3 right = Vector3(cos(yawRad), 0.0f, -sin(yawRad));

    Vector3 movement = ((right * controller->GetNamedAxis("Sidestep") * strafeMulti * playerSpeed) + (forward * -controller->GetNamedAxis("Forward")) * playerSpeed) * moveScale;
    movement.y = -1000.0f * (0.5f + inAirCount); // arteficial gravity

    btVector3 rotatedMovement(movement.x, movement.y, movement.z);

    //jumps allow for held input up to maxJumpTime
    bool jumpPressed = controller->GetNamedButton("JumpButton");
    if (!inAir && jumpPressed) {
        spaceCount += dt;
        rotatedMovement.setY(rotatedMovement.getY() + (jumpHeight));
        if (spaceCount > maxJumpTime) {
            inAir = true;
        }
    }
    else if (!jumpPressed && spaceCount > 0) {
        inAir = true;
    }

    //Get player and just below player positions as btVector3
    btTransform transformPlayer = player->GetPhysicsObject()->GetRigidBody()->getWorldTransform();
    btVector3 btPlayerPos = transformPlayer.getOrigin();
    btVector3 btBelowPlayerPos = btPlayerPos;
    btBelowPlayerPos.setY(btBelowPlayerPos.getY() - 4.2);

    //create a callback, can use closest result or all results
    btCollisionWorld::ClosestRayResultCallback callback(btPlayerPos, btBelowPlayerPos);

    //send ray
    bulletWorld->rayTest(btPlayerPos, btBelowPlayerPos, callback);

    if (callback.hasHit()) {
        btVector3 hitPoint = callback.m_hitPointWorld; // option to use hit point in world position
        const btRigidBody* hitBody = btRigidBody::upcast(callback.m_collisionObject); // cast collision into rigid body type
        if (hitBody) { // found an object, have it's rigid body
            inAir = false;
            spaceCount = 0;
            inAirCount = 0;
        }
    }
    else { // did not find
        if (!jumpPressed) {
            inAir = true;
        }
        inAirCount += dt;
    }

    player->GetPhysicsObject()->GetRigidBody()->applyCentralImpulse(rotatedMovement*dt);
}