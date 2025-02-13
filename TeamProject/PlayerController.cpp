#include "PlayerController.h"

using namespace NCL;
using namespace CSC8503;

Vector2 PlayerController::getDirectionalInput() const
{
    Vector2 raw(controller->GetAnalogue(Controller::AnalogueControl::MoveSidestep), controller->GetAnalogue(Controller::AnalogueControl::MoveForward));
    float magnitude = Vector::Length(raw);
    return magnitude <= 1.0f ? raw : raw / magnitude;
}

void PlayerController::Initialise() {
    rb = player->GetPhysicsObject()->GetRigidBody();
}

void PlayerController::UpdateMovement(float dt) {
    transformPlayer = rb->getWorldTransform();
    btPlayerPos = transformPlayer.getOrigin();
    btVector3 upDirection = CalculateUpDirection();

    //camera yaw
    yaw = fmod(yaw - controller->GetAnalogue(Controller::AnalogueControl::LookX) + 360.0f, 360.0f);
    if (!thirdPerson) camera->SetYaw(yaw);

    if (controller->GetDigital(Controller::DigitalControl::Fire) && shotTimer >= shotCooldown) {
        ShootBullet();
        shotTimer = 0.0f;
    }
    else {
        shotTimer += dt;
    }

    //sliding/floor detection
    HandleSliding(dt);
    HandleCrouching(dt);
    if ((isSliding||slideTransition) && !isCrouching) return;
    float roll = CalculateRoll();
    camera->SetRoll(roll);
    //player rotation
    btQuaternion playerRotation(btVector3(0, 1, 0), Maths::DegreesToRadians(yaw));
    btQuaternion playerRotation2(btVector3(0, 0, 1), Maths::DegreesToRadians(roll));
    transformPlayer.setRotation(playerRotation2* playerRotation);
    rb->setWorldTransform(transformPlayer);

    //camera follows player, lowers if crouching

    btTransform transformPlayerMotion;
    player->GetPhysicsObject()->GetMotionState()->getWorldTransform(transformPlayerMotion);
    btVector3 playerCamPos = transformPlayerMotion.getOrigin();
    playerCamPos += upDirection * (isCrouching ? std::lerp(cameraHeight, crouchHeight, btMin(currentCrouchingTimer / crouchingTime, 1.0f)) : std::lerp(crouchHeight, cameraHeight, btMin(currentStandingTimer / crouchingTime, 1.0f)));
    if (!slideTransition && !thirdPerson) {
        camera->SetPosition(playerCamPos);
        SetGunTransform();
    }

    //finds player forward and right vectors
    btMatrix3x3 rotationMatrix(playerRotation2 * playerRotation);
    btVector3 forward = rotationMatrix * btVector3(0, 0, -1);
    btVector3 up = rotationMatrix * btVector3(0, 1, 0);
    btVector3 right = rotationMatrix * btVector3(1, 0, 0);
    //movement based on all the multipliers combined
    Vector2 directionalInput = getDirectionalInput();
    bool sprinting = controller->GetDigital(Controller::DigitalControl::Sprint);
    float forwardMovement = directionalInput.y;
    float moveMulti = playerSpeed * (sprinting ? sprintMulti : 1) * (isCrouching ? crouchMulti : 1) * (player->getCollided()==0 ? airMulti : 1) ;
    forwardMovement *= (forwardMovement <= 0) ? backwardsMulti : 1;
    btVector3 movement = (right * directionalInput.x * strafeMulti * moveMulti) +(forward * forwardMovement * moveMulti);
    movement += upDirection * -(gravityScale * dt);

    // jump input
    if (controller->GetDigital(Controller::DigitalControl::Jump) && player->getCollided()) {
        movement += (upDirection * jumpHeight);
        // movement.setY(jumpHeight);
        player->setCollided(0);
    }
    else {
       
        movement += upDirection.absolute() * rb->getLinearVelocity();
    }

    // Apply the movement according to the computed directions
    if (player->getCollided() == 0) {
       // movement += forward * rb->getLinearVelocity();
      //  movement += right * rb->getLinearVelocity();
    }


    rb->setLinearVelocity(movement);
    rb->activate();
}




//attaches gun to the camera position/rotation
void PlayerController::SetGunTransform() {
    float pitchRadians = Maths::DegreesToRadians(camera->GetPitch());
    float yawRadians = Maths::DegreesToRadians(camera->GetYaw());
    float rollRadians = Maths::DegreesToRadians(camera->GetRoll());
    btQuaternion yawQuat(btVector3(0, 1, 0), yawRadians);
    btQuaternion pitchQuat(btVector3(1, 0, 0), pitchRadians);
    btQuaternion rollQuat(btVector3(0, 0, 1), rollRadians);
    btQuaternion gunRotation = rollQuat *yawQuat * pitchQuat; // Yaw first, then pitch

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
    float rollRadians = Maths::DegreesToRadians(camera->GetRoll());
    btQuaternion yawQuat(btVector3(0, 1, 0), yawRadians);
    btQuaternion pitchQuat(btVector3(1, 0, 0), pitchRadians);
    btQuaternion rollQuat(btVector3(0, 0, 1), rollRadians);
    btQuaternion bulletRotation = rollQuat * yawQuat * pitchQuat;

    // Compute rotation matrix
    btMatrix3x3 rotationMatrix(bulletRotation);
    btVector3 adjustedOffset = rotationMatrix * bulletCameraOffset;
    btVector3 forwardDir = rotationMatrix * btVector3(0, 0, -1);
    btVector3 rightDir = rotationMatrix * btVector3(1, 0, 0);
    btVector3 bulletPos = camera->GetPosition() + adjustedOffset;

    Paintball* paintball = new Paintball();
    paintball->Initialise(player,bulletWorld);
    Vector3 bulletSize(1, 1, 1);
    paintball->setInitialPosition(bulletPos);
    paintball->setRenderScale(bulletSize);
    paintball->SetRenderObject(new RenderObject(
        paintball,
        resourceManager->getMeshes().get("Sphere.msh"),
        resourceManager->getTextures().get("checkerboard.png"),
        resourceManager->getShaders().get(Shader::Default)
    ));
    paintball->SetPhysicsObject(new PhysicsObject(paintball));
    paintball->GetRenderObject()->SetColour(Vector4(rand() % 2, rand() % 2, rand() % 2, 1));
    btCollisionShape* shape = new btSphereShape(1);
    shape->setMargin(0.01f);
    paintball->GetPhysicsObject()->InitBulletPhysics(bulletWorld, shape, 1.0f);
    world->AddGameObject(paintball);

    btVector3 playerVelocity = rb->getLinearVelocity();
    float forwardSpeed = forwardDir.dot(playerVelocity);
    float rightSpeed = rightDir.dot(playerVelocity);
    btVector3 adjustedPlayerVelocity = (forwardDir * forwardSpeed) + (rightDir * rightSpeed * playerVelocityStrafeInherit);
    btVector3 bulletVelocity = adjustedPlayerVelocity + (forwardDir * bulletSpeed);

    // Apply impulse
    paintball->GetPhysicsObject()->GetRigidBody()->applyCentralImpulse(bulletVelocity);
    paintball->GetPhysicsObject()->GetRigidBody()->activate();
}


//transitions states between standing and crouching
void PlayerController::HandleCrouching(float dt) {
    if (isSliding) {
        return;
    }
    if ((crouching && !controller->GetDigital(Controller::DigitalControl::Crouch)) || slideTransition) {
        crouching = CheckCeling();
    }
    else {
        crouching = controller->GetDigital(Controller::DigitalControl::Crouch);
    }

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


//uses ray to detect if the player is blocked from standing
bool PlayerController::CheckCeling() {
    btVector3 btBelowPlayerPos = btPlayerPos;
    btBelowPlayerPos += (CalculateUpDirection() * 4.1f);
  //  btBelowPlayerPos.setY(btBelowPlayerPos.getY() + 4.1f);
    btCollisionWorld::ClosestRayResultCallback callback(btPlayerPos, btBelowPlayerPos);
    bulletWorld->rayTest(btPlayerPos, btBelowPlayerPos, callback);
    if (callback.hasHit()) {
        return true;
    }
    else {
        return false;
    }
}


//transitions states between standing and sliding, also handles physics for while sliding
void PlayerController::HandleSliding(float dt) {
    bool crouching = controller->GetDigital(Controller::DigitalControl::Crouch);
    bool sprinting = controller->GetDigital(Controller::DigitalControl::Sprint);
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

    if ((slideTransition || isSliding) && !isCrouching) {
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
        //CheckFloor(dt);
        btVector3 pastMovement = rb->getLinearVelocity();
        pastMovement += CalculateUpDirection() * (-gravityScale * dt);
    //    pastMovement.setY(pastMovement.getY() - ( * (player->getCollided() == 0 ? 1 : 10)));
        rb->setLinearVelocity(pastMovement);
        rb->activate();
    }
}


btVector3 PlayerController::CalculateUpDirection() {
    float t = fmod(worldRotation, 1.0f);
    int index = static_cast<int>(worldRotation);
    btVector3 directions[] = {
        btVector3(0, 1, 0),
        btVector3(-1, 0, 0),
        btVector3(0, -1, 0),
        btVector3(1, 0, 0)
    };
    btVector3 currentDir = directions[index % 4];
    btVector3 nextDir = directions[(index + 1) % 4];

    // Interpolate between currentDir and nextDir
    btVector3 upDirection = currentDir.lerp(nextDir, t);
    upDirection.normalize();

    return upDirection;
}

float PlayerController::CalculateRoll() {
    float t = fmod(worldRotation, 1.0f);
    int index = static_cast<int>(worldRotation);
    float rolls[] = {
        0.0f,
        90.0f,
        180.0f,
        270.0f
    };
    float currentRoll = rolls[index % 4];
    float nextRoll = rolls[(index + 1) % 4];

    // Handle wrap-around from 270.0f to 0.0f
    if (currentRoll == 270.0f && nextRoll == 0.0f) {
        nextRoll = 360.0f;
    }

    // Interpolate between currentRoll and nextRoll
    float roll = currentRoll + t * (nextRoll - currentRoll);

    return roll;
}
