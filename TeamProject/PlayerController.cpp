#include "PlayerController.h"
#include "AudioEngine.h"
#include "TutorialGame.h"
#include "Multiplayer/GamePackets.hpp"
#include "Health.h"
#include "AnimationObject.h"

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
    scoreboard = std::make_unique<Scoreboard>();
    overheat = std::make_unique<Overheat>();
    renderer->AddUiElement(crosshair.get());
	renderer->AddUiElement(scoreboard.get());
    renderer->AddUiElement(overheat.get());
    crosshair->SetActive(true);
    overheat->SetActive(true);
    renderer->SetVignetteOn(true);
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

    if (crosshair && overheat) {
        crosshair->Animate(dt);
        overheat->Animate(dt);
    }
    HandleYaw();
    SpecialTypeCalculations();
    HandleSliding(dt);

    if (inAirTime > 0) {
        inAirTime -= dt;
    }
    if (isSliding || slideTransition) {
        player->SetAnimationState(AnimationState::SLIDING);
        return;
    }
    RotationCalculations();
 
    CameraMovement();
    GroundNormalCalculations();
    MovementCalculations(dt);
    HandleJumping();
    HandleHurtEffects();

    previousVelocity = rb->getLinearVelocity();
    rb->setLinearVelocity(movement);
    rb->activate();
    HandleShooting(dt);
	ToggleScoreboard();
}


void PlayerController::HandleShooting(float dt) {

    if (controller->GetDigital(Controller::DigitalControl::Fire) && overheat->CanFire()) {
        FireShot(dt);
        if (!firing) {
            crosshair->fire();
            overheat->fire();
            firing = true;
        }
    }
    else {
        if (firing) {
            player->updateLaser(btVector3(0,0,0), btVector3(0,0,0));
            crosshair->stopFiring();
            overheat->stopFiring();
            firing = false;
            player->GetAttackAttrib()->Hit(nullptr);
        }
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
    btVector3 adjustedOffset = rotationMatrix * gunCameraOffset; // Apply rotation to the offset
    
    // Calculate forward direction based on where crosshair lands.
    std::optional<ShotInfo> crosshairRay = Shoot::GetInstance()->RayClosest(
        camera->GetPosition(), forwardDir
    );
    if (crosshairRay == std::nullopt) return;

    btVector3 crosshairLookPoint = crosshairRay->hitPos;
    btVector3 startPos = player->getGun()->GetPhysicsObject()->GetRigidBody()->getWorldTransform().getOrigin() + adjustedOffset;
    btVector3 gunForwardDir = (crosshairLookPoint - startPos).normalized();

    std::optional<ShotInfo> info = Shoot::GetInstance()->ShootBulletPlayer(startPos, gunForwardDir, bulletRotation, dt, player->GetWorldID());
    if (info != std::nullopt) {
        player->GetLaser()->SetCollisionNormal(info.value().hitNormal);
        player->updateLaser(player->getGun()->GetPhysicsObject()->GetRigidBody()->getWorldTransform().getOrigin() + adjustedOffset, info.value().hitPos);

        // Shooting at a player.
        if (info->hitObj->getType() == GameObject::Type::Player) {
            player->GetAttackAttrib()->Hit(((PlayerObject*)info->hitObj)->GetHealthAttrib());
        }

        // Shooting at an AI.
        else if (info->hitObj->getType() == GameObject::Type::AI) {
            player->GetAttackAttrib()->Hit(((Wanderer*)info->hitObj)->GetHealthAttrib());
        }

        else player->GetAttackAttrib()->Hit(nullptr);
    }
    else player->GetAttackAttrib()->Hit(nullptr);
}


// finds surface normal of floor below
btVector3 PlayerController::FindFloorNormal() {
    btVector3 btBelowPlayerPos = btPlayerPos;
    btBelowPlayerPos -= (upDirection * 30);
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
    bool slidingCondition = crouching && sprinting;

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
        playerPos += upDirection * std::lerp(isSliding ? camOffset.y() : slidingCameraHeight, isSliding ? slidingCameraHeight : camOffset.y(), slideFactor);
        if (!thirdPerson) {
            camera->SetPosition(playerPos);
            player->SetGunTransform(camera->GetPitch(), camera->GetYaw(), playerPos);
        }
        btVector3 pastMovement = rb->getLinearVelocity();
        pastMovement += upDirection * -(gravityScale * dt);
        previousVelocity = rb->getLinearVelocity();
        rb->setLinearVelocity(pastMovement);
        rb->activate();
        HandleShooting(dt);
    }
}


void PlayerController::SpecialTypeCalculations() {
       switch (player->getCollisionType())

    {
    case GameObject::Type::Default:
        onIce = false;
        break;
    case GameObject::Type::JumpPad: {
        onIce = false;
        btVector3 normal = player->getCollisionNormal();
        float dotProduct = normal.dot(upDirection.absolute());
        btVector3 movement = (player->getCollisionJumpPadStrength() * -player->getCollisionNormal());
        rb->setLinearVelocity(btVector3(0, 0, 0));
        inAirTime = 0.2f;
        rb->applyCentralImpulse(movement);
        break;
    } case GameObject::Type::Slime: {
        onIce = false;
        if (inAirTime <= 0) {
            btVector3 normal = player->getCollisionNormal();
            float dampening = 0.85f;
            btVector3 reflectedVelocity = previousVelocity - (2 * previousVelocity.dot(normal) * normal);
            if (fabs(10 * previousVelocity.dot(normal)) > 800.0f) {
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
    playerCamPos += camOffset.x() * right;
    playerCamPos += camOffset.y() * up;
    playerCamPos += camOffset.z() * forward;
    if (!slideTransition && !thirdPerson) {
        camera->SetPosition(playerCamPos);
        player->SetGunTransform(camera->GetPitch(), camera->GetYaw(), playerCamPos);
    }
};

//if on ground, movement based on floor angle
void PlayerController::GroundNormalCalculations() {
    if (player->getCollided() > 0 && inAirTime <=0.0f) {
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
    float moveMulti = playerSpeed * (sprinting ? sprintMulti : 1) * ((player->getCollided() <= 0|| inAirTime > 0.0f) ? airMulti : 1);
    forwardMovement *= (forwardMovement <= 0) ? backwardsMulti : 1;
    movement = (right * directionalInput.x * strafeMulti * moveMulti) + (forward * forwardMovement * moveMulti);
    if (player->getCollided() <= 0 || inAirTime > 0.0f || onIce) {
        movement *= (airMulti * dt);
        movement += rb->getLinearVelocity();
    }
    if (player->getCollided() > 0) {
        airTimeCounter = 0;
    }

    //animations
    if (player->getCollided() <= 0) {
        airTimeCounter += dt;
        if (airTimeCounter > 0.20f) { //0.05
            player->SetAnimationState(AnimationState::FALLING);
        }
    }
    else if (directionalInput.y >= 0.01f) {
        player->SetAnimationState(sprinting ? AnimationState::SPRINTING_FORWARD : AnimationState::WALKING_FORWARD);
    }
    else if (directionalInput.y <= -0.01f) {
        player->SetAnimationState(sprinting ? AnimationState::SPRINTING_BACK : AnimationState::WALKING_BACK);
    }
    else if (directionalInput.x >= 0.01f) {
        player->SetAnimationState(sprinting ? AnimationState::SPRINTING_RIGHT : AnimationState::WALKING_RIGHT);
    }
    else if (directionalInput.x <= -0.01f) {
        player->SetAnimationState(sprinting ? AnimationState::SPRINTING_LEFT : AnimationState::WALKING_LEFT);
    }
    else {
        player->SetAnimationState(AnimationState::IDLE);
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
        inAirTime = 0.2f;
    }
    if (inAirTime > 0) {
        if (controller->GetDigital(Controller::DigitalControl::Sprint)) {
            player->SetAnimationState(AnimationState::JUMPING_SPRINT);
        }
        else {
            player->SetAnimationState(AnimationState::JUMPING_STANDING);
        }
        
    }
};

void PlayerController::HandleHurtEffects() {
    HealthAttrib* health = player->GetHealthAttrib();
    float healthLossPercent = (health->GetMaxHealth() - health->GetCurrentHealth()) / health->GetMaxHealth();
    renderer->SetVignetteIntesnity((healthLossPercent));
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

void PlayerController::ToggleScoreboard() {
	if (controller->GetDigital(Controller::DigitalControl::Scoreboard)) {
		scoreboardActive = !scoreboardActive;
        scoreboard->SetActive(scoreboardActive);
	}
}
