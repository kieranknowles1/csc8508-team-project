#include "PlayerController.h"

using namespace NCL;
using namespace CSC8503;

// Helper function to print btVector3
std::ostream& operator<<(std::ostream& os, const btVector3& vec) {
    os << "(" << vec.getX() << ", " << vec.getY() << ", " << vec.getZ() << ")";
    return os;
}


void PlayerController::Initialise() {
    rb = player->GetPhysicsObject()->GetRigidBody();
}

void PlayerController::UpdateMovement(float dt) {
    transformPlayer = rb->getWorldTransform();
    btPlayerPos = transformPlayer.getOrigin();
    upDirection = CalculateUpDirection();
    rightDirection = CalculateRightDirection(upDirection);
    forwardDirection = CalculateForwardDirection(upDirection, rightDirection);
    player->setUpDirection(upDirection);


    roll = CalculateRoll();
    yaw = fmod(yaw - controller->GetAnalogue(Controller::AnalogueControl::LookX) + 360.0f, 360.0f);
    float pitch = CalculatePitch();
    camera->setPitchOffset(pitch);

    if (pitch == 0.0f) {
        if (!thirdPerson) camera->SetYaw(yaw);
    }
    else {
        float temp = roll;
        if (pitch > 0) {
            roll = yaw;
            yaw = roll;
        }
        else{
            roll = -yaw;
            yaw = -roll;
         }
     
    }

    camera->SetRoll(roll);


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

    //player rotation
    btQuaternion playerRotation(btVector3(0, 1, 0), Maths::DegreesToRadians(yaw));
    btQuaternion playerRotation2(btVector3(0, 0, 1), Maths::DegreesToRadians(roll));
    btQuaternion playerRotation3(btVector3(1, 0, 0), Maths::DegreesToRadians(pitch));
    transformPlayer.setRotation(playerRotation2 * playerRotation3 * playerRotation);
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
    btMatrix3x3 rotationMatrix(playerRotation2 * playerRotation3 * playerRotation);
    btVector3 forward = rotationMatrix * btVector3(0, 0, -1);
    btVector3 up = rotationMatrix * btVector3(0, 1, 0);
    btVector3 right = rotationMatrix * btVector3(1, 0, 0);

    //if on ground, movement based on floor angle
    if (player->getCollided() > 0) {
        btVector3 groundNormal = FindFloorNormal();
        if (groundNormal != btVector3(0, 0, 0)) {
            float cosAngleThreshold = cos(btRadians(60.0f));
            float dotProduct = groundNormal.dot(upDirection.absolute());
            if (dotProduct >= cosAngleThreshold) {
                btVector3 slopeForward = forward - (forward.dot(groundNormal)) * groundNormal;
                slopeForward.normalize();
                btVector3 slopeRight = right - (right.dot(groundNormal)) * groundNormal;
                slopeRight.normalize();
                if (slopeForward.length2() > SIMD_EPSILON && slopeRight.length2() > SIMD_EPSILON) {
                    forward = slopeForward;
                    right = slopeRight;
                }
            }
        }
    }

    //movement based on all the multipliers combined
    Vector2 directionalInput = getDirectionalInput();
    bool sprinting = controller->GetDigital(Controller::DigitalControl::Sprint);
    float forwardMovement = directionalInput.y;
    float moveMulti = playerSpeed * (sprinting ? sprintMulti : 1) * (isCrouching ? crouchMulti : 1) * (player->getCollided() <= 0 ? airMulti : 1);
    forwardMovement *= (forwardMovement <= 0) ? backwardsMulti : 1;
    btVector3 movement = (right * directionalInput.x * strafeMulti * moveMulti ) +(forward * forwardMovement * moveMulti);

    if (inAirTime > 0) {
        player->setCollided(0);
        inAirTime -= dt;
    }
    if (player->getCollided() <= 0) {
       // std::cout << "IN AIR" << std::endl;
        movement *= airMulti;
        movement += rb->getLinearVelocity();
    }
    else {
       // std::cout << "ON FLOOR" << std::endl;
    }
   // std::cout << player->getCollided() << std::endl;
    movement += upDirection * -(gravityScale * dt);

    // jump input
    if (controller->GetDigital(Controller::DigitalControl::Jump) && player->getCollided()) {
        movement += (FindFloorNormal() * jumpHeight);
        player->setCollided(0);
        inAirTime = 0.2f;
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
        nullptr,
        resourceManager->getShaders().get(Shader::Default)
    ));
    paintball->GetRenderObject()->SetIsFlat(true);
    paintball->SetPhysicsObject(new PhysicsObject(paintball));
    paintball->GetRenderObject()->SetColour(Vector4(rand() % 2, rand() % 2, rand() % 2, 1));
    btCollisionShape* shape = new btSphereShape(1);
    shape->setMargin(0.01f);
    paintball->GetPhysicsObject()->InitBulletPhysics(bulletWorld, shape, 1.0f);
    world->AddGameObject(paintball);

    btVector3 playerVelocity = rb->getLinearVelocity();
    float forwardSpeed = forwardDir.dot(playerVelocity);
    float rightSpeed = rightDir.dot(playerVelocity);
    btVector3 adjustedPlayerVelocity = (forwardDir * forwardSpeed) + (rightDirection * rightSpeed * playerVelocityStrafeInherit);
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
    btBelowPlayerPos += (upDirection * 4.1f);
    btCollisionWorld::ClosestRayResultCallback callback(btPlayerPos, btBelowPlayerPos);
    bulletWorld->rayTest(btPlayerPos, btBelowPlayerPos, callback);
    if (callback.hasHit()) {
        return true;
    }
    else {
        return false;
    }
}

// finds surface normal of floor below
btVector3 PlayerController::FindFloorNormal() {
    btVector3 btBelowPlayerPos = btPlayerPos;
    btBelowPlayerPos -= (upDirection * 8);
    btCollisionWorld::ClosestRayResultCallback callback(btPlayerPos, btBelowPlayerPos);
    bulletWorld->rayTest(btPlayerPos, btBelowPlayerPos, callback);
    if (callback.hasHit()) {
        return callback.m_hitNormalWorld;
    }
    else {
        return btVector3();
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

        btQuaternion playerRotation1(btVector3(0, 1, 0), Maths::DegreesToRadians(yaw));
        btQuaternion playerRotation2(btVector3(0, 0, 1), Maths::DegreesToRadians(roll));
        //finds player forward and right vectors
        btMatrix3x3 rotationMatrix(playerRotation2 * playerRotation1);
        btVector3 forward = rotationMatrix * btVector3(0, 0, -1);
        btVector3 right = rotationMatrix * btVector3(1, 0, 0);
        btQuaternion playerRotationX(right, Maths::DegreesToRadians(std::lerp(isSliding ? 0 : slidingAngle, isSliding ? slidingAngle : 0, slideFactor)));

        btQuaternion playerRotation = playerRotationX* playerRotation2 * playerRotation1;
        transformPlayer.setRotation(playerRotation);
        player->GetPhysicsObject()->GetRigidBody()->setWorldTransform(transformPlayer);
        btTransform transformPlayerMotion;
        player->GetPhysicsObject()->GetMotionState()->getWorldTransform(transformPlayerMotion);
        btVector3 playerPos = transformPlayerMotion.getOrigin();

        playerPos -= forward * std::lerp(isSliding ? 0 : slidingCameraBackwards, isSliding ? slidingCameraBackwards : 0, slideFactor);
        playerPos += upDirection * std::lerp(isSliding ? cameraHeight : slidingCameraHeight, isSliding ? slidingCameraHeight : cameraHeight, slideFactor);
        if (!thirdPerson) {
            camera->SetPosition(playerPos);
            SetGunTransform();
        }
        //CheckFloor(dt);
        btVector3 pastMovement = rb->getLinearVelocity();
        pastMovement += upDirection * -(gravityScale * dt);
    //    pastMovement.setY(pastMovement.getY() - ( * (player->getCollided() == 0 ? 1 : 10)));
        rb->setLinearVelocity(pastMovement);
        rb->activate();
    }
}


btVector3 PlayerController::CalculateUpDirection() {
    float t = fmod(worldRotation, 1.0f);
    int index = floor(worldRotation);
    btVector3 directions[] = {
        btVector3(0, 1, 0),
        btVector3(-1, 0, 0),
        btVector3(0, -1, 0),
        btVector3(1, 0, 0),
        btVector3(0, 0, 1),
        btVector3(0, 0, -1)
    };
    btVector3 currentDir = directions[index % 6];
    btVector3 nextDir = directions[(index + 1) % 6];

    // Interpolate between currentDir and nextDir
    btVector3 upDir = currentDir.lerp(nextDir, t);
    upDir.normalize();
    return upDir;
}

btVector3 PlayerController::CalculateRightDirection(btVector3 upDir) {
    btVector3 forward = btVector3(0, 0, 1);
    upDir.normalize();
    float dotProduct = upDir.dot(forward);
    if (fabs(dotProduct) > 0.9999f) { // If almost collinear
        forward = btVector3(0, 1, 0);
    }
    btVector3 rightDirection = upDir.cross(forward);
    rightDirection.normalize();

    return rightDirection;
}


btVector3 PlayerController::CalculateForwardDirection(btVector3 upDir,btVector3 rightDir) {
    btVector3 forwardDirection = rightDir.cross(upDir);
    forwardDirection.normalize();
    return forwardDirection;
}


float PlayerController::CalculateRoll() {
    float t = fmod(worldRotation, 1.0f);
    int index = floor(worldRotation);
    float rolls[] = {
        0.0f,
        90.0f,
        180.0f,
        270.0f,
        0.0f,
        0.0f
    };
    float currentRoll = rolls[index % 6];
    float nextRoll = rolls[(index + 1) % 6];

    // Handle wrap-around from 270.0f to 0.0f
    if (currentRoll == 270.0f && nextRoll == 0.0f) {
        nextRoll = 360.0f;
    }

    // Interpolate between currentRoll and nextRoll
    float roll = currentRoll + t * (nextRoll - currentRoll);

    return roll;
}

float PlayerController::CalculatePitch() {
    float t = fmod(worldRotation, 1.0f);
    int index = floor(worldRotation);
    float pitches[] = {
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        90.0f,
        -90.0f
   };
    float currentPitch = pitches[index % 6];
    float nextPitch = pitches[(index + 1) % 6];
    // Interpolate between currentRoll and nextRoll
    float pitch = currentPitch + t * (nextPitch - currentPitch);

    return pitch;
}


Vector2 PlayerController::getDirectionalInput() const
{
    Vector2 raw(controller->GetAnalogue(Controller::AnalogueControl::MoveSidestep), controller->GetAnalogue(Controller::AnalogueControl::MoveForward));
    float magnitude = Vector::Length(raw);
    return magnitude <= 1.0f ? raw : raw / magnitude;
}