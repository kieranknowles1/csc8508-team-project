#include "PlayerObject.h"
#include "TutorialGame.h"
#include "Multiplayer/GamePackets.hpp"
#include "Multiplayer/Server.hpp"
#include "Health.h"
#include "Respawn.h"
#include "Score.h"

#include <memory>

using namespace WorldState;
using namespace NCL;
using namespace CSC8503;


PlayerObject::PlayerObject() {
    health = std::make_unique<HealthAttrib>(this);
    health->SetMaxHealth(PLAYER_HEALTH);
    health->SetCurrentHealth(PLAYER_HEALTH);
    health->SetRegenerationDelay(4.0f);
    health->SetRegenerationRate(25.0f);
    health->SetInvulnerableWindow(1.0f);

    attack = std::make_unique<AttackAttrib>();
    attack->SetDamageType(DamageType::CONTINUOUS);
    attack->SetDamageAmount(200.0f);
    attack->SetHealthAttrib(health.get());

    score = std::make_unique<ScoreAttrib>(this);
}


PlayerObject::~PlayerObject() {
    if (laser && TutorialGame::getInstance()) {
        TutorialGame::getInstance()->delayedRemoveObject(laser);
    }
    laser = nullptr;
    delete animationObject;
}

void PlayerObject::SetColor(btVector4 color) {
    GameObject::GetRenderObject()->SetColour(color);
    gun->GetRenderObject()->SetColour(color);
    laser->SetColor(color);
}

void PlayerObject::Update(float dt) {
    attack->Update(dt);
    health->Update(dt);

    upDirection = CalculateUpDirection(dt);

    rightDirection = CalculateRightDirection(upDirection);
    forwardDirection = CalculateForwardDirection(upDirection, rightDirection);
    updateGravity(dt);
    //Animation: 
    CorrectAnimation();

    if (renderObject->GetAnimation()) {
        renderObject->GetAnimation()->UpdateAnimation(dt);
    }

    elapsedTime += dt;
}

void PlayerObject::UpdateWorldState() {
    score->UpdateWorldState();
    health->UpdateWorldState();
    attack->UpdateWorldState();
    GameObject::UpdateWorldState();

    auto [writeState, lock] = GetWorldStates()->GetWriteState();
    std::unique_lock stateLock(writeState->Lock());

    writeState->UpdateState(StateType::UpVector, upDirection);
    int animationInt = static_cast<int>(animationState);
    writeState->UpdateState(StateType::Animation, animationInt);
}

void PlayerObject::UpdateFromWorldState(float tickProgress) {
    score->UpdateFromWorldState(tickProgress);
    attack->UpdateFromWorldState(tickProgress);
    health->UpdateFromWorldState(tickProgress);
    GameObject::UpdateFromWorldState(tickProgress);

    std::function lerp = [](float x, float y, float w) { return x + ((y - x) * w); };

    auto [current, currentLock] = GetWorldStates()->GetCurrentState();
    auto [read, readLock] = GetWorldStates()->GetReadState();

    StateValue currentUpVectorValue;
    StateValue targetUpVectorValue;
    StateValue animationValue;

    std::shared_lock currentStateLock = current->Lock_Shared();
    std::shared_lock readStateLock = read->Lock_Shared();

    bool hasCurrentUpVector = current->ReadState(StateType::UpVector, &currentUpVectorValue);
    bool hasTargetUpVector = read->ReadState(StateType::UpVector, &targetUpVectorValue);
    bool hasAnimation = current->ReadState(StateType::Animation, &animationValue);

    currentStateLock.unlock();
    readStateLock.unlock();

    currentLock.unlock();
    readLock.unlock();

    // Interpolate up vector.
    if (hasCurrentUpVector && hasTargetUpVector) {
        btVector3 currentUpVector = std::get<btVector3>(currentUpVectorValue);
        btVector3 targetUpVector = std::get<btVector3>(targetUpVectorValue);
        btVector3 interpolated = btVector3(
            lerp(currentUpVector.x(), targetUpVector.x(), tickProgress),
            lerp(currentUpVector.y(), targetUpVector.y(), tickProgress),
            lerp(currentUpVector.z(), targetUpVector.z(), tickProgress)
        );

        setUpDirection(interpolated);
    }
    if (hasAnimation) {
        int animNumber = std::get<int>(animationValue);
        animationState = static_cast<AnimationState>(animNumber);
    }
}

std::vector<std::shared_ptr<Packet::Packet>> PlayerObject::CreatePackets(int sequenceNum) {
    std::vector<std::shared_ptr<Packet::Packet>> packets = GameObject::CreatePackets(sequenceNum);
    std::vector<std::shared_ptr<Packet::Packet>> damagePackets = attack->CreatePackets(sequenceNum);
    std::vector<std::shared_ptr<Packet::Packet>> healthPackets = health->CreatePackets(sequenceNum);
    std::vector<std::shared_ptr<Packet::Packet>> scorePackets = score->CreatePackets(sequenceNum);

    packets.insert(packets.end(), damagePackets.begin(), damagePackets.end());
    packets.insert(packets.end(), healthPackets.begin(), healthPackets.end());
    packets.insert(packets.end(), scorePackets.begin(), scorePackets.end());

    auto [read, readLock] = GetWorldStates()->GetReadState();

    StateValue upVector;
    StateValue animation;

    std::shared_lock readStateLock = read->Lock_Shared();

    bool hasUpVector = read->ReadState(StateType::UpVector, &upVector);
    bool hasAnimation = read->ReadState(StateType::Animation, &animation);

    readStateLock.unlock();
    readLock.unlock();

    if (hasUpVector) {
        packets.push_back(std::move(std::make_shared<Packet::ObjectChangeGravityPacket>(
            GetWorldID(),
            std::get<btVector3>(upVector),
            sequenceNum
        )));
    }
    if (hasAnimation) {
        packets.push_back(std::move(std::make_shared<Packet::PlayerAnimationPacket>(
            GetWorldID(),
            std::get<int>(animation),
            sequenceNum
        )));
    }
    return std::move(packets);
}

void PlayerObject::OnCollisionEnter(const CollisionInfo& collisionInfo){
	btVector3 playerPos = this->GetPhysicsObject()->GetRigidBody()->getWorldTransform().getOrigin();
	btVector3 objPos = collisionInfo.contactPointA;
	btVector3 direction = (objPos - playerPos).normalized();
	float dot = direction.dot(-upDirection);
	float angle = acos(dot) * (180.0f / SIMD_PI);

    // set special type collision
    if (angle <= 25.0f) {
        if (collisionInfo.otherObject->getType() == Type::Ice) {
            collisionType = Type::Ice;
        }
    }
    if (collisionInfo.otherObject->getType() == Type::JumpPad || collisionInfo.otherObject->getType() == Type::Slime) {
        collisionNormal = collisionInfo.contactNormal;
        collisionPoint = collisionInfo.contactPointA;
        collisionType = collisionInfo.otherObject->getType();
        jumpPadHeight = collisionInfo.otherObject->getJumpPadStrength();
    }
}


void PlayerObject::OnCollisionExit(const CollisionInfo& collisionInfo){
	auto it = std::find(collidedObjects.begin(), collidedObjects.end(), collisionInfo.otherObject);
	if (it != collidedObjects.end()) {
        resetCollisionType();
		collidedObjects.erase(it);
		if (collided > 0) {
			collided--;
		}
	}
}

void PlayerObject::OnCollisionStay(const CollisionInfo& collision){
	btVector3 playerPos = this->GetPhysicsObject()->GetRigidBody()->getWorldTransform().getOrigin();
	btVector3 objPos = collision.contactPointA;
	btVector3 direction = (objPos - playerPos).normalized();
	float dot = direction.dot(-upDirection);
	float angle = acos(dot) * (180.0f / SIMD_PI);
	auto it = std::find(collidedObjects.begin(), collidedObjects.end(), collision.otherObject);
	if (it != collidedObjects.end()) { // contains already
		if (angle > 25.0f) { // now too steep for floor
			collidedObjects.erase(it);
			if (collided > 0) {
				collided--;
			}
		}
	}
	else if (angle <= 25.0f) {
        // not counted as floor yet
		collided++;
		collidedObjects.push_back(collision.otherObject);
	}

	// set special type collision
    if (angle <= 25.0f) {
        if (collisionType == Type::Default && collision.otherObject->getType() == Type::Ice) {
            collisionType = Type::Ice;
        }
    }
}


void PlayerObject::updateGravity(float dt) {
	btVector3 movement = this->GetPhysicsObject()->GetRigidBody()->getLinearVelocity();
	movement += upDirection * -(gravityScale * dt);
	this->GetPhysicsObject()->GetRigidBody()->setLinearVelocity(movement);
}


// world rotate things
void PlayerObject::Rotate(bool positive, bool rolling, float yaw) {
    if (rotationChanging) return;
    btVector3 rightDirections = (rolling ? CalculateForwardFromYaw(yaw) : CalculateRightFromYaw(yaw));
    btQuaternion pitchQuat(rightDirections, Maths::DegreesToRadians(positive ? 90 : -90));
    targetWorldRotation = quatRotate(pitchQuat, upDirection);
    targetcamRotOffset = pitchQuat * oldcamRotOffset; // Apply rotation correctly
    rotationChanging = true;
}


btVector3 PlayerObject::CalculateUpDirection(float dt) {
    btVector3 upDir;
    if (!rotationChanging) {
        upDir = targetWorldRotation;
        camRotOffset = targetcamRotOffset;

    }
    else if (rotateTimer <= rotateTime && rotationChanging) {
        rotateTimer += dt;
        upDir = lerp(oldWorldRotation, targetWorldRotation, rotateTimer / rotateTime);
        camRotOffset = (oldcamRotOffset.slerp(targetcamRotOffset, rotateTimer / rotateTime));
    }
    else {
        upDir = targetWorldRotation;
        camRotOffset = targetcamRotOffset;
        oldcamRotOffset = targetcamRotOffset;
        oldWorldRotation = targetWorldRotation;
        rotationChanging = false;
        rotateTimer = 0.0f;
    }
    upDir.normalize();
    return upDir;
}


//Most things past here are basically just helper functions
btVector3 PlayerObject::CalculateForwardFromYaw(float yaw) {
    int snappedYaw = static_cast<int>((yaw + 45) / 90) * 90 % 360;
    btVector3 forwd = btVector3(0, 0, 0);
    switch (snappedYaw) {
    case 0: {
        forwd = btVector3(0, 0, 1);
        break;
    }
    case 90: {
        forwd = btVector3(1, 0, 0);
        break;
    }
    case 180: {
        forwd = btVector3(0, 0, -1);
        break;
    }
    case 270: {
        forwd = btVector3(-1, 0, 0);
        break;
    }
    }
    btVector3 baseForward = quatRotate(oldcamRotOffset, forwd);
    return baseForward;
}

btVector3 PlayerObject::CalculateRightFromYaw(float yaw) {
    int snappedYaw = static_cast<int>((yaw + 45) / 90) * 90 % 360;
    btVector3 right = btVector3(0, 0, 0);
    switch (snappedYaw) {
    case 0: {
        right = btVector3(1, 0, 0);
        break;
    }
    case 90: {
        right = btVector3(0, 0, -1);
        break;
    }
    case 180: {
        right = btVector3(-1, 0, 0);
        break;
    }
    case 270: {
        right = btVector3(0, 0, 1);
        break;
    }
    }
    btVector3 baseForward = quatRotate(oldcamRotOffset, right);
    return baseForward;
}

btVector3 PlayerObject::CalculateRightDirection(btVector3 upDir) {
    btVector3 forward = btVector3(0, 0, 1);
    if (fabs(upDir.dot(forward)) > 0.999f) {
        forward = btVector3(0, 1, 0);
    }
    btVector3 rightDirection = upDir.cross(forward);
    rightDirection.normalize();
    return rightDirection;
}

btVector3 PlayerObject::CalculateForwardDirection(btVector3 upDir, btVector3 rightDir) {
    btVector3 forwardDirection = rightDir.cross(upDir);
    forwardDirection.normalize();
    return forwardDirection;
}

//attaches gun to the camera position/rotation
void PlayerObject::SetGunTransform(float pitch, float yaw, btVector3 camPos) {
    float pitchRadians = Maths::DegreesToRadians(pitch);
    float yawRadians = Maths::DegreesToRadians(yaw);
    btQuaternion yawQuat(btVector3(0, 1, 0), yawRadians);
    btQuaternion pitchQuat(btVector3(1, 0, 0), pitchRadians);

    btQuaternion extraOffset = btQuaternion(Maths::DegreesToRadians(90), 0, 0);
    btQuaternion gunRotation = camRotOffset * yawQuat * pitchQuat * extraOffset; // Yaw first, then pitch

    btMatrix3x3 rotationMatrixCam(gunRotation);
    btVector3 adjustedOffset = rotationMatrixCam * gunCameraOffset; // Apply rotation to the offset

    // Gun Animation starts here
    float speed = GetPhysicsObject()->GetRigidBody()->getLinearVelocity().length();

    if (speed > 0.1f) {
        float frequency = 1.0f; // Adjust for faster/slower blobbing
        float amplitude = 0.01f; // Adjust for bigger/smaller blobbing
        float time = Maths::DegreesToRadians(elapsedTime * 360.0f); // Convert to radians

        // Apply bobbing effect
        float bobbingOffsetY = sin(time * frequency) * (amplitude * 0.2) * (speed / 5.0f);
        float bobbingOffsetX = cos(time * frequency * 0.5f) * (amplitude * 0.1f) * (speed / 5.0f);

        adjustedOffset += btVector3(bobbingOffsetX, bobbingOffsetY, 0);
    }


    btTransform& transformGun = gun->GetPhysicsObject()->GetRigidBody()->getWorldTransform();
    transformGun.setOrigin(camPos + adjustedOffset);
    transformGun.setRotation(gunRotation);
}


void PlayerObject::CorrectAnimation() {
    //to be called in update. Checks if the animation matches the state, if not sets it to the right animation. Hopefully means it is only set when state changes.#
    if (renderObject->GetAnimation() != animationObject->getAnimation(animationState)) {
        renderObject->SetAnimation(animationObject->getAnimation(animationState));
    }
}