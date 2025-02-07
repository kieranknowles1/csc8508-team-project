#include "PlayerController.h"


using namespace NCL;
using namespace CSC8503;

void PlayerController::UpdateMovement(float dt) {
    transformPlayer = rb->getWorldTransform();
    btPlayerPos = transformPlayer.getOrigin();


    //camera yaw
    yaw = fmod(yaw - controller->GetNamedAxis("XLook") + 360.0f, 360.0f);
    if (!thirdPerson) camera->SetYaw(yaw);

    //sliding/floor detection
    HandleSliding(dt);
    CheckFloor(dt);
    if (isSliding) return;

    //player rotation
    btQuaternion playerRotation(btVector3(0, 1, 0), Maths::DegreesToRadians(yaw));
    transformPlayer.setRotation(playerRotation);
    rb->setWorldTransform(transformPlayer);

    //camera follows player, lowers if crouching
    HandleCrouching(dt);
    btTransform transformPlayerMotion;
    player->GetPhysicsObject()->GetMotionState()->getWorldTransform(transformPlayerMotion);
    btVector3 playerCamPos = transformPlayerMotion.getOrigin();
    playerCamPos.setY(playerCamPos.getY() + (isCrouching ? std::lerp(cameraHeight, crouchHeight, std::min(currentCrouchingTimer / crouchingTime, 1.0f)) : std::lerp(crouchHeight, cameraHeight, std::min(currentStandingTimer / crouchingTime, 1.0f))));
    if (!slideTransition && !thirdPerson) {
        camera->SetPosition(playerCamPos);
        SetGunTransform();
    }

    //finds player forward and right vectors
    btMatrix3x3 rotationMatrix(playerRotation);
    btVector3 forward = rotationMatrix * btVector3(0, 0, -1);
    btVector3 right = rotationMatrix * btVector3(1, 0, 0);

    //movement based on all the multipliers combined
    bool diag = controller->GetNamedAxis("Sidestep") && controller->GetNamedAxis("Forward");
    float moveScale = diag ? diagonalMulti : 1.0f;
    bool sprinting = controller->GetNamedButton("Sprint");
    float forwardMovement = controller->GetNamedAxis("Forward");
    float moveMulti = playerSpeed * moveScale * (sprinting ? sprintMulti : 1) * (isCrouching ? crouchMulti : 1);
    forwardMovement *= (forwardMovement <= 0) ? backwardsMulti : 1;
    Vector3 movement = (right * controller->GetNamedAxis("Sidestep") * strafeMulti * moveMulti) +(forward * forwardMovement * moveMulti);

    //gravity is handled here manually, dampening is too high for gravity to work in bullet
    movement.y = -gravityScale * (0.5f + inAirCount);

    // jump input
    if (!inAir && controller->GetNamedButton("Jump")) {
        spaceCount += dt;
        movement.y += jumpHeight;
        if (spaceCount > maxJumpTime) inAir = true;
    }
    else if (!controller->GetNamedButton("Jump") && spaceCount > 0) {
        inAir = true;
    }

    rb->applyCentralImpulse(btVector3(movement.x, movement.y, movement.z) * dt);
}



//uses ray to detect if the player is standing on an object
void PlayerController::CheckFloor(float dt) {
    bool jumpPressed = controller->GetNamedButton("Jump");
    btVector3 btBelowPlayerPos = btPlayerPos;
    btBelowPlayerPos.setY(btBelowPlayerPos.getY() - 4.2);
    btCollisionWorld::ClosestRayResultCallback callback(btPlayerPos, btBelowPlayerPos);
    bulletWorld->rayTest(btPlayerPos, btBelowPlayerPos, callback);

    if (callback.hasHit()) {
        inAir = false;
        spaceCount = 0;
        inAirCount = 0;
    }
    else { 
        if (!jumpPressed) {
            inAir = true;
        }
        inAirCount += dt;
    }
}


//attaches gun to the camera position/rotation
void PlayerController::SetGunTransform() {
    float pitchRadians = Maths::DegreesToRadians(camera->GetPitch());
    float yawRadians = Maths::DegreesToRadians(camera->GetYaw());

    btQuaternion yawQuat(btVector3(0, 1, 0), yawRadians);
    btQuaternion pitchQuat(btVector3(1, 0, 0), pitchRadians);
    btQuaternion gunRotation = yawQuat * pitchQuat; // Yaw first, then pitch

    btMatrix3x3 rotationMatrixCam(gunRotation);
    btVector3 adjustedOffset = rotationMatrixCam * cameraOffset; // Apply rotation to the offset

    transformGun = gun->GetPhysicsObject()->GetRigidBody()->getWorldTransform();
    btGunPos = camera->GetPosition() + adjustedOffset; // Offset from camera position
    transformGun.setOrigin(btGunPos);
    transformGun.setRotation(gunRotation);

    gun->GetPhysicsObject()->GetRigidBody()->setWorldTransform(transformGun);
}


//transitions states between standing and crouching
void PlayerController::HandleCrouching(float dt) {
    bool crouching = controller->GetNamedButton("Crouch");
    crouchTransition = crouching ? (currentCrouchingTimer < crouchingTime) : (currentStandingTimer < crouchingTime);

    if (crouching) {
        isCrouching = true;
        currentStandingTimer = 0;
        currentCrouchingTimer = std::min(currentCrouchingTimer + dt, crouchingTime);
        currentHeight = std::lerp(standingHeight, crouchingHeight, currentCrouchingTimer / crouchingTime);
    }
    else {
        isCrouching = false;
        currentCrouchingTimer = 0;
        currentStandingTimer = std::min(currentStandingTimer + dt, crouchingTime);
        currentHeight = std::lerp(crouchingHeight, standingHeight, currentStandingTimer / crouchingTime);
    }

    if (crouchTransition) {
        Vector3 currentScale = player->getRenderScale();
        currentScale.y = currentHeight;
        player->setRenderScale(currentScale);
        btCollisionShape* shape = new btCapsuleShape(radius, currentHeight);
        player->GetPhysicsObject()->GetRigidBody()->setCollisionShape(shape);
    }
}


//transitions states between standing and sliding, also handles physics for while sliding
void PlayerController::HandleSliding(float dt) {
    bool crouching = controller->GetNamedButton("Crouch");
    bool sprinting = controller->GetNamedButton("Sprint");
    bool slidingCondition = crouching && sprinting && !isCrouching;

    slideTransition = slidingCondition
        ? (currentSlidingTimer < slidingTime)
        : (currentStandingSlideTimer < slidingTime);

    isSliding = slidingCondition;

    if (isSliding) {
        rb->setDamping(slidingDampening, 1);
        currentStandingSlideTimer = 0;
        currentSlidingTimer = std::min(currentSlidingTimer + dt, slidingTime);
    }
    else {
        rb->setDamping(normalDampening, 1);
        currentSlidingTimer = 0;
        currentStandingSlideTimer = std::min(currentStandingSlideTimer + dt, slidingTime);
    }

    if (slideTransition || isSliding) {
        float slideFactor = isSliding ? std::min(currentSlidingTimer / slidingTime, 1.0f) : std::min(currentStandingSlideTimer / slidingTime, 1.0f);

        btQuaternion playerRotation(btVector3(0, 1, 0), Maths::DegreesToRadians(yaw));
        btQuaternion playerRotationX(btVector3(1, 0, 0),Maths::DegreesToRadians(std::lerp(isSliding ? 0 : slidingAngle, isSliding ? slidingAngle : 0, slideFactor)));

        transformPlayer.setRotation(playerRotation * playerRotationX);
        player->GetPhysicsObject()->GetRigidBody()->setWorldTransform(transformPlayer);

        btMatrix3x3 rotationMatrix(playerRotation);
        btVector3 forward = rotationMatrix * btVector3(0, 0, -1);
        btTransform transformPlayerMotion;
        player->GetPhysicsObject()->GetMotionState()->getWorldTransform(transformPlayerMotion);
        btVector3 playerPos = transformPlayerMotion.getOrigin();

        playerPos -= forward * std::lerp(isSliding ? 0 : slidingCameraBackwards, isSliding ? slidingCameraBackwards : 0, slideFactor);
        playerPos.setY(playerPos.getY() + std::lerp(isSliding ? cameraHeight : slidingCameraHeight, isSliding ? slidingCameraHeight : cameraHeight, slideFactor));
        if (!thirdPerson) {
            camera->SetPosition(playerPos);
            SetGunTransform();
        }

        btVector3 gravity(0, (- gravityScale * (0.5f + inAirCount))* slidingDampening, 0);
        rb->applyCentralImpulse(gravity * dt);
    }
}
