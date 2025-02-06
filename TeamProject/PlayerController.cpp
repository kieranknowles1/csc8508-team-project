#include "PlayerController.h"


using namespace NCL;
using namespace CSC8503;

void PlayerController::UpdateMovement(float dt) {

    // set position to be equal to player
    btTransform transformPlayerMotion;
    player->GetPhysicsObject()->GetMotionState()->getWorldTransform(transformPlayerMotion);
    btVector3 playerPos = transformPlayerMotion.getOrigin();
    playerPos.setY(playerPos.getY() + 3);
    camera->SetPosition(playerPos);


    // Update yaw based on controller input
    yaw -= controller->GetNamedAxis("XLook");
    if (yaw < 0) {
        yaw += 360.0f;
    }
    if (yaw > 360.0f) {
        yaw -= 360.0f;
    }
    camera->SetYaw(yaw);

    bool diag = (controller->GetNamedAxis("Sidestep") != 0 && controller->GetNamedAxis("Forward") != 0);
    float moveScale = diag ? 0.5f : 1.0f;

    btQuaternion playerRotation(btVector3(0, 1, 0), Maths::DegreesToRadians(yaw));
    btTransform transformPlayer = player->GetPhysicsObject()->GetRigidBody()->getWorldTransform();
    transformPlayer.setRotation(playerRotation);
    player->GetPhysicsObject()->GetRigidBody()->setWorldTransform(transformPlayer);

    btMatrix3x3 rotationMatrix(playerRotation);
    btVector3 forward = rotationMatrix * btVector3(0, 0, -1); 
    btVector3 right = rotationMatrix * btVector3(1, 0, 0);   
    Vector3 forwardVector(forward.x(), forward.y(), forward.z());
    Vector3 rightVector(right.x(), right.y(), right.z());

    bool sprinting = controller->GetNamedButton("Sprint");
    float moveMulti = playerSpeed * moveScale * (sprinting?sprintMulti:1);
    float forwardMovement = controller->GetNamedAxis("Forward");
    if (forwardMovement <= 0) { forwardMovement *= backwardsMulti; }
    Vector3 movement = ((rightVector * controller->GetNamedAxis("Sidestep") * strafeMulti * moveMulti) + (forwardVector * forwardMovement * moveMulti));

    movement.y = -gravityScale * (0.5f + inAirCount); // arteficial gravity

    btVector3 rotatedMovement(movement.x, movement.y, movement.z);

    //jumps allow for held input up to maxJumpTime
    bool jumpPressed = controller->GetNamedButton("Jump");
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
        const GameObject* hitObj = static_cast<GameObject*>(hitBody->getUserPointer()); // cast to find GameObject*

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