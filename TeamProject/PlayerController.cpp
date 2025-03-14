#include "PlayerController.h"
#include "AudioEngine.h"
#include "TutorialGame.h"


using namespace NCL;
using namespace CSC8503;

// Helper function to print btVector3
std::ostream& operator<<(std::ostream& os, const btVector3& vec) {
    os << "(" << vec.getX() << ", " << vec.getY() << ", " << vec.getZ() << ")";
    return os;
}

// Helper function to print btQuaternion
std::ostream& operator<<(std::ostream& os, const btQuaternion& vec) {
    os << "(" << vec.getX() << ", " << vec.getY() << ", " << vec.getZ()  << ", "<< vec.getW() << ")";
    return os;
}


void PlayerController::Initialise() {
    rb = player->GetPhysicsObject()->GetRigidBody();
    debugDrawer = bulletWorld->getDebugDrawer();
    crosshair = std::make_unique<Crosshair>();
    renderer->AddUiElement(crosshair.get());
    crosshair->SetActive(true);
}

btVector3 GetEulerAngles(btQuaternion quat) {
    btScalar roll2, pitch2, yaw2;
    quat.getEulerZYX(yaw2, pitch2, roll2);
    return btVector3(pitch2, roll2, yaw2) * (180.0f / SIMD_PI);
}


void PlayerController::UpdateMovement(float dt) {
    transformPlayer = rb->getWorldTransform();
    btPlayerPos = transformPlayer.getOrigin();
    GetAllDirections();
    HandleShooting(dt);
    if (crosshair) {
        crosshair->Animate(dt);
    }
    HandleYaw();
    SpecialTypeCalculations();
    HandleSliding(dt);
    HandleCrouching(dt);

    if (inAirTime > 0) {
        player->setCollided(0);
        inAirTime -= dt;
    }
    if ((isSliding||slideTransition) && !isCrouching) return;
    RotationCalculations();
    CameraMovement();
    GroundNormalCalculations();
    MovementCalculations(dt);
    HandleJumping();
    HandleHurtEffects();

    previousVelocity = rb->getLinearVelocity();
    rb->setLinearVelocity(movement);
    rb->activate();
}


void PlayerController::HandleShooting(float dt) {
    if (controller->GetDigital(Controller::DigitalControl::Fire) && shotTimer >= shotCooldown) {
        FireShot(dt);
        crosshair->fire();
        shotTimer = 0.0f;
    }
    else {
        shotTimer += dt;
    }
}



void PlayerController::FireShot(float dt) {
    // Convert camera pitch & yaw to radians
    float pitchRadians = Maths::DegreesToRadians(camera->GetPitch());
    float yawRadians = Maths::DegreesToRadians(yaw);
    btQuaternion yawQuat(btVector3(0, 1, 0), yawRadians);
    btQuaternion pitchQuat(btVector3(1, 0, 0), pitchRadians);
    btQuaternion bulletRotation = camRotOffset * yawQuat * pitchQuat;
    btMatrix3x3 rotationMatrix(bulletRotation);
    btVector3 forwardDir = rotationMatrix * btVector3(0, 0, -1);

   Shoot::GetInstance()->ShootBulletPlayer(camera->GetPosition(), forwardDir, bulletRotation, dt);
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
    btBelowPlayerPos -= (upDirection * 16);
    btCollisionWorld::ClosestRayResultCallback callback(btPlayerPos, btBelowPlayerPos);
    bulletWorld->rayTest(btPlayerPos, btBelowPlayerPos, callback);
    if (callback.hasHit()) {
        return callback.m_hitNormalWorld;
    }
    else {
        return upDirection;
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

        //finds player forward and right vectors
        btMatrix3x3 rotationMatrix(camRotOffset * playerRotation1);
        btVector3 forward = rotationMatrix * btVector3(0, 0, -1);
        btVector3 right = rotationMatrix * btVector3(1, 0, 0);
        btQuaternion playerRotationX(right, Maths::DegreesToRadians(std::lerp(isSliding ? 0 : slidingAngle, isSliding ? slidingAngle : 0, slideFactor)));

        btQuaternion playerRotation = playerRotationX* camRotOffset * playerRotation1;
        transformPlayer.setRotation(playerRotation);
        player->GetPhysicsObject()->GetRigidBody()->setWorldTransform(transformPlayer);
        btTransform transformPlayerMotion;
        player->GetPhysicsObject()->GetMotionState()->getWorldTransform(transformPlayerMotion);
        btVector3 playerPos = transformPlayerMotion.getOrigin();

        playerPos -= forward * std::lerp(isSliding ? 0 : slidingCameraBackwards, isSliding ? slidingCameraBackwards : 0, slideFactor);
        playerPos += upDirection * std::lerp(isSliding ? cameraHeight : slidingCameraHeight, isSliding ? slidingCameraHeight : cameraHeight, slideFactor);
        if (!thirdPerson) {
            camera->SetPosition(playerPos);
            player->SetGunTransform(camera->GetPitch(), camera->GetYaw(), playerPos);
        }
        //CheckFloor(dt);
        btVector3 pastMovement = rb->getLinearVelocity();
        pastMovement += upDirection * -(gravityScale * dt);
        previousVelocity = rb->getLinearVelocity();
        rb->setLinearVelocity(pastMovement);
        rb->activate();
    }
}


void PlayerController::SpecialTypeCalculations() {
       switch (player->getCollisionType())

    {
    case GameObject::Type::Default:
        onIce = false;
        break;
    case GameObject::Type::JumpPad: {
        btVector3 normal = player->getCollisionNormal();
        float dotProduct = normal.dot(upDirection.absolute());
        btVector3 movement = btVector3(0, 0, 0);
        movement += (bouncePadHeight * -player->getCollisionNormal());
        rb->setLinearVelocity(btVector3(0, 0, 0));
        player->setCollided(0);
        inAirTime = 0.2f;
        rb->applyCentralImpulse(movement);
        break;
    } case GameObject::Type::Slime: {
        if (inAirTime <= 0) {
            btVector3 normal = player->getCollisionNormal();
            float dampening = 0.85f;
            btVector3 reflectedVelocity = previousVelocity - (2 * previousVelocity.dot(normal) * normal);
            if (fabs(10 * previousVelocity.dot(normal)) <= 0.25f) {
                btVector3 direction = (player->getCollisionPoint() - transformPlayer.getOrigin()).normalized();
                float dot = direction.dot(-upDirection);
                float angle = acos(dot) * (180.0f / SIMD_PI);
                if (angle <= 25.0f) { // come to rest on floor
                    player->setCollided(1);
                    break;
                }
            }
            else {
                reflectedVelocity *= dampening;
                inAirTime = 0.1f;
                rb->setLinearVelocity(reflectedVelocity);
            }
        }
        break;
    }
    case GameObject::Type::Ice: {
        onIce = true;
        break;
    }
    default:
        break;
    }
    player->resetCollisionType();
}

void PlayerController::HandleYaw() {
    yaw = fmod(yaw - controller->GetAnalogue(Controller::AnalogueControl::LookX) + 360.0f, 360.0f);
    camera->SetYaw(yaw);
    btVector3 rot = GetEulerAngles(camRotOffset);
    camera->setRotation(rot);
}

void PlayerController::RotationCalculations() {
    //player rotation based on yaw
    btQuaternion playerYaw = btQuaternion(btVector3(0, 1, 0), Maths::DegreesToRadians(yaw));
    btQuaternion finalRotation = camRotOffset * playerYaw;
    transformPlayer.setRotation(finalRotation);

    rb->setWorldTransform(transformPlayer);
    //finds player forward and right vectors
    btMatrix3x3 rotationMatrix(finalRotation);
    forward = rotationMatrix * btVector3(0, 0, -1);
    up = rotationMatrix * btVector3(0, 1, 0);
    right = rotationMatrix * btVector3(1, 0, 0);

};

//camera follows player, lowers if crouching, don't change if third person
void PlayerController::CameraMovement() {
    btTransform transformPlayerMotion;
    player->GetPhysicsObject()->GetMotionState()->getWorldTransform(transformPlayerMotion);
    btVector3 playerCamPos = transformPlayerMotion.getOrigin();
    playerCamPos += upDirection * (isCrouching ? std::lerp(cameraHeight, crouchHeight, btMin(currentCrouchingTimer / crouchingTime, 1.0f)) : std::lerp(crouchHeight, cameraHeight, btMin(currentStandingTimer / crouchingTime, 1.0f)));
    if (!slideTransition && !thirdPerson) {
        camera->SetPosition(playerCamPos);
        player->SetGunTransform(camera->GetPitch(), camera->GetYaw(), playerCamPos);
    }
};

//if on ground, movement based on floor angle
void PlayerController::GroundNormalCalculations() {
    if (player->getCollided() > 0) {
        btVector3 groundNormal = FindFloorNormal();
        if (groundNormal != btVector3(0, 0, 0)) {
            float cosAngleThreshold = cos(btRadians(50.0f));
            float dotProduct = groundNormal.dot(upDirection.absolute());
            if (fabs(dotProduct) >= cosAngleThreshold && fabs(dotProduct) <= 1.0f) {
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
};

//movement based on all the multipliers combined
void PlayerController::MovementCalculations(float dt) {
    Vector2 directionalInput = getDirectionalInput();
    bool sprinting = controller->GetDigital(Controller::DigitalControl::Sprint);
    float forwardMovement = directionalInput.y;
    float moveMulti = playerSpeed * (sprinting ? sprintMulti : 1) * (isCrouching ? crouchMulti : 1) * (player->getCollided() <= 0 ? airMulti : 1);
    forwardMovement *= (forwardMovement <= 0) ? backwardsMulti : 1;
    movement = (right * directionalInput.x * strafeMulti * moveMulti) + (forward * forwardMovement * moveMulti);

    if (player->getCollided() <= 0 || onIce) {
        movement *= (airMulti * dt);
        movement += rb->getLinearVelocity();
    }
};


void PlayerController::HandleJumping() {
    if (controller->GetDigital(Controller::DigitalControl::Jump) && player->getCollided() && inAirTime <= 0) {
        audioEngine.PlaySounds("jump.wav", camera->GetPosition(), 0.0f);
        btVector3 normal = FindFloorNormal();
        float dotProduct = normal.dot(upDirection.absolute());
        if (fabs(dotProduct <= 1)) {
            movement += (jumpHeight * normal);
        }
        else {
            movement += (jumpHeight * upDirection);
        }
        player->setCollided(0);
        inAirTime = 0.2f;
    }
};

void PlayerController::HandleHurtEffects() {
    float healthLossPercent = (player->GetMaxHealth() - player->health) / player->GetMaxHealth();
    if (healthLossPercent <= 0.001f) {
        renderer->SetVignetteOn(false);
    }
    else {
        renderer->SetVignetteOn(true);
        renderer->SetVignetteIntesnity((1.75f * healthLossPercent));
    }

}

void PlayerController::GetAllDirections() {
    upDirection = player->getUpDirection();
    rightDirection = player->getForwardDirection();
    forwardDirection = player->getRightDirection();
    camRotOffset = player->getCamOffset();

    //Update FMod listener each frame so audio is correctly positioned
    audioEngine.Set3dListenerAndOrientation(camera->GetPosition(), forwardDirection, upDirection);
}

Vector2 PlayerController::getDirectionalInput() const
{
    Vector2 raw(controller->GetAnalogue(Controller::AnalogueControl::MoveSidestep), controller->GetAnalogue(Controller::AnalogueControl::MoveForward));
    float magnitude = Vector::Length(raw);
    return magnitude <= 1.0f ? raw : raw / magnitude;
}
