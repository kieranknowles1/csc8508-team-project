#include "PlayerObject.h"
#include "TutorialGame.h"
#include "Multiplayer/GamePackets.hpp"

#include <memory>

using namespace NCL;
using namespace CSC8503;

void PlayerObject::Update(float dt) {
    upDirection = CalculateUpDirection(dt);
    rightDirection = CalculateRightDirection(upDirection);
    forwardDirection = CalculateForwardDirection(upDirection, rightDirection);
    updateGravity(dt);

    //Animation: 
    if (animated == true) {
        renderObject->GetAnimation()->UpdateAnimation(dt);
    }

    elapsedTime += dt;

    // 2 seconds before healing.
    if (elapsedTime - lastHit > 4.0f) {
        health += 25 * dt;
        if (health > maxHealth) health = maxHealth;
    }

    if (TutorialGame::GetServerInstance().has_value() && (owner == TutorialGame::GetUser())) {
        std::shared_ptr<Packet::DeltaPacket> deltaPacket = std::make_shared<Packet::DeltaPacket>(
            worldID,
            GetPhysicsObject()->GetRigidBody()->getLinearVelocity(),
            GetPhysicsObject()->GetRigidBody()->getAngularVelocity(),
            GetLastPacketSequence((uint8_t)Packet::PacketType::DELTA) + 1
        );
        UpdatePacketSequence((uint8_t)Packet::PacketType::DELTA, deltaPacket->GetSequenceNumber());
        TutorialGame::GetServerInstance()->Broadcast(deltaPacket);

        btTransform transform = GetPhysicsObject()->GetRigidBody()->getWorldTransform();
        std::shared_ptr<Packet::PositionPacket> positionPacket = std::make_shared<Packet::PositionPacket>(
            worldID,
            transform.getOrigin(),
            transform.getRotation(),
            GetLastPacketSequence((uint8_t)Packet::PacketType::POSITION) + 1
        );
        UpdatePacketSequence((uint8_t)Packet::PacketType::POSITION, positionPacket->GetSequenceNumber());
        TutorialGame::GetServerInstance()->Broadcast(positionPacket);

        std::shared_ptr<Packet::ObjectChangeGravityPacket> gravityPacket = std::make_shared<Packet::ObjectChangeGravityPacket>(
            worldID,
            upDirection,
            GetLastPacketSequence((uint8_t)Packet::PacketType::OBJECT_CHANGE_GRAVITY) + 1
        );
        UpdatePacketSequence((uint8_t)Packet::PacketType::OBJECT_CHANGE_GRAVITY, gravityPacket->GetSequenceNumber());
        TutorialGame::GetServerInstance()->Broadcast(gravityPacket);
    }
    std::cout << "ANIMATED STATE: " << AnimationNames[animationState] << std::endl;
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


    btTransform transformGun = gun->GetPhysicsObject()->GetRigidBody()->getWorldTransform();
    transformGun.setOrigin(camPos + adjustedOffset);
    transformGun.setRotation(gunRotation);

    gun->GetPhysicsObject()->GetRigidBody()->setWorldTransform(transformGun);
}
