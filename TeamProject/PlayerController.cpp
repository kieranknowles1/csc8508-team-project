#include "PlayerController.h"

using namespace NCL;
using namespace CSC8503;

void PlayerController::Initialise() {
    rb = player->GetPhysicsObject()->GetRigidBody();
    sphereMesh = renderer->LoadMesh("sphere.msh");
    basicTex = renderer->LoadTexture("checkerboard.png");
    basicShader = renderer->LoadShader("scene.vert", "scene.frag");
}

void PlayerController::UpdateMovement(float dt) {
    transformPlayer = rb->getWorldTransform();
    btPlayerPos = transformPlayer.getOrigin();

    //camera yaw
    yaw = fmod(yaw - controller->GetNamedAxis("XLook") + 360.0f, 360.0f);
    if (!thirdPerson) camera->SetYaw(yaw);

    if (controller->GetNamedButton("LeftMouseButton") && shotTimer >= shotCooldown) {
        ShootBullet();
        shotTimer = 0.0f;
    }
    else {
        shotTimer += dt;
    }

    //sliding/floor detection
    HandleSliding(dt);
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
    playerCamPos.setY(playerCamPos.getY() + (isCrouching ? std::lerp(cameraHeight, crouchHeight, btMin(currentCrouchingTimer / crouchingTime, 1.0f)) : std::lerp(crouchHeight, cameraHeight, btMin(currentStandingTimer / crouchingTime, 1.0f))));
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
    float moveMulti = playerSpeed * moveScale * (sprinting ? sprintMulti : 1) * (isCrouching ? crouchMulti : 1) * (inAir ? airMulti : 1) ;
    forwardMovement *= (forwardMovement <= 0) ? backwardsMulti : 1;
    btVector3 movement = (right * controller->GetNamedAxis("Sidestep") * strafeMulti * moveMulti) +(forward * forwardMovement * moveMulti);
    movement.setY(movement.getY() - gravityScale);
    movement = movement * dt * speed;
    

    // jump input
    if (!inAir && controller->GetNamedButton("Jump")) {
        rb->setDamping(jumpDampening, 1);
        movement.setY(movement.getY() + jumpHeight);
        inAir = true;
        inAirCount = 0.2f;
        std::cout << "JUMP";
    }
    else {
        inAirCount = btMax(0.0f, inAirCount - dt);
    }
    CheckFloor(dt);
    if (inAir) {
        movement.setX(rb->getLinearVelocity().getX() + (movement.getX() * airMulti));
        movement.setZ(rb->getLinearVelocity().getZ() + (movement.getZ() * airMulti));
    }
    movement.setY(movement.getY() + rb->getLinearVelocity().getY());
    rb->setLinearVelocity(movement);
}



//uses ray to detect if the player is standing on an object
void PlayerController::CheckFloor(float dt) {
    btVector3 btBelowPlayerPos = btPlayerPos;
    btBelowPlayerPos.setY(btBelowPlayerPos.getY() - 4.0f);
    btCollisionWorld::ClosestRayResultCallback callback(btPlayerPos, btBelowPlayerPos);
    bulletWorld->rayTest(btPlayerPos, btBelowPlayerPos, callback);

    if (callback.hasHit() && inAirCount <= 0) {
        inAir = false;
    }
    else { 
        inAir = true;
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
    btVector3 adjustedOffset = rotationMatrixCam * gunCameraOffset; // Apply rotation to the offset

    transformGun = gun->GetPhysicsObject()->GetRigidBody()->getWorldTransform();
    btGunPos = camera->GetPosition() + adjustedOffset; // Offset from camera position
    transformGun.setOrigin(btGunPos);
    transformGun.setRotation(gunRotation);

    gun->GetPhysicsObject()->GetRigidBody()->setWorldTransform(transformGun);
}

void PlayerController::ShootBullet() {
    // Convert camera pitch & yaw to radians
    float pitchRadians = Maths::DegreesToRadians(camera->GetPitch());
    float yawRadians = Maths::DegreesToRadians(camera->GetYaw());
    btQuaternion yawQuat(btVector3(0, 1, 0), yawRadians);
    btQuaternion pitchQuat(btVector3(1, 0, 0), pitchRadians);
    btQuaternion bulletRotation = yawQuat * pitchQuat;

    // Compute rotation matrix
    btMatrix3x3 rotationMatrix(bulletRotation);
    btVector3 adjustedOffset = rotationMatrix * bulletCameraOffset;
    btVector3 forwardDir = rotationMatrix * btVector3(0, 0, -1);
    btVector3 rightDir = rotationMatrix * btVector3(1, 0, 0);
    btVector3 bulletPos = camera->GetPosition() + adjustedOffset;

    Bullet* bullet = new Bullet();
    bullet->Initialise(player,bulletWorld);
    Vector3 bulletSize(1, 1, 1);
    bullet->setInitialPosition(bulletPos);
    bullet->setRenderScale(bulletSize);
    bullet->SetRenderObject(new RenderObject(bullet, sphereMesh, basicTex, basicShader));
    bullet->SetPhysicsObject(new PhysicsObject(bullet));
    bullet->GetRenderObject()->SetColour(Vector4(rand() % 2, rand() % 2, rand() % 2, 1));
    btCollisionShape* shape = new btSphereShape(1);
    shape->setMargin(0.01f);
    bullet->GetPhysicsObject()->InitBulletPhysics(bulletWorld, shape, 1.0f);
    world->AddGameObject(bullet);

    btVector3 playerVelocity = rb->getLinearVelocity();
    float forwardSpeed = forwardDir.dot(playerVelocity);
    float rightSpeed = rightDir.dot(playerVelocity);
    btVector3 adjustedPlayerVelocity = (forwardDir * forwardSpeed) + (rightDir * rightSpeed * playerVelocityStrafeInherit);
    btVector3 bulletVelocity = adjustedPlayerVelocity + (forwardDir * bulletSpeed);

    // Apply impulse
    bullet->GetPhysicsObject()->GetRigidBody()->applyCentralImpulse(bulletVelocity);
}


//transitions states between standing and crouching
void PlayerController::HandleCrouching(float dt) {
    bool crouching = controller->GetNamedButton("Crouch");
    crouchTransition = crouching ? (currentCrouchingTimer < crouchingTime) : (currentStandingTimer < crouchingTime);

    if (crouching) {
        isCrouching = true;
        currentStandingTimer = 0;
        currentCrouchingTimer = btMin(currentCrouchingTimer + dt, crouchingTime);
        currentHeight = std::lerp(standingHeight, crouchingHeight, currentCrouchingTimer / crouchingTime);
    }
    else {
        isCrouching = false;
        currentCrouchingTimer = 0;
        currentStandingTimer = btMin(currentStandingTimer + dt, crouchingTime);
        currentHeight = std::lerp(crouchingHeight, standingHeight, currentStandingTimer / crouchingTime);
    }

    if (crouchTransition) {
        Vector3 currentScale = player->getRenderScale();
        currentScale.y = (currentHeight+2)*0.7;
        player->setRenderScale(currentScale);

        btCollisionShape* shape = player->GetPhysicsObject()->GetRigidBody()->getCollisionShape();
        shape->setLocalScaling(btVector3(1, currentHeight/standingHeight, 1));
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
        currentStandingSlideTimer = 0;
        currentSlidingTimer = btMin(currentSlidingTimer + dt, slidingTime);
    }
    else {
        currentSlidingTimer = 0;
        currentStandingSlideTimer = btMin(currentStandingSlideTimer + dt, slidingTime);
    }

    if (slideTransition || isSliding) {
        float slideFactor = isSliding ? btMin(currentSlidingTimer / slidingTime, 1.0f) : btMin(currentStandingSlideTimer / slidingTime, 1.0f);

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
        CheckFloor(dt);
        btVector3 pastMovement = rb->getLinearVelocity();
        pastMovement.setY(pastMovement.getY() - (gravityScale * dt * speed * (inAir?1:10)));
        rb->setLinearVelocity(pastMovement);
    }
}
