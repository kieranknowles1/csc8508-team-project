#include "PlayerObject.h"

using namespace NCL;
using namespace CSC8503;

void PlayerObject::Update(float dt) {
    upDirection = CalculateUpDirection(dt);
    rightDirection = CalculateRightDirection(upDirection);
    forwardDirection = CalculateForwardDirection(upDirection, rightDirection);
}

void PlayerObject::OnCollisionEnter(const CollisionInfo& collisionInfo){
	if (collisionInfo.otherObject->getIsPaintball()) return;
	btVector3 playerPos = this->GetPhysicsObject()->GetRigidBody()->getWorldTransform().getOrigin();
	btVector3 objPos = collisionInfo.contactPointA;
	btVector3 direction = (objPos - playerPos).normalized();
	float dot = direction.dot(-upDirection);
	float angle = acos(dot) * (180.0f / SIMD_PI);
	if (angle <= 25.0f) {
		collided++;
		collidedObjects.push_back(collisionInfo.otherObject);
	}

	// set special type collision
    collisionType = collisionInfo.otherObject->getType();
    if (collisionInfo.otherObject->getType() == GameObject::Type::JumpPad || collisionInfo.otherObject->getType() == GameObject::Type::Slime) {
		collisionNormal = collisionInfo.contactNormal;
		collisionPoint = collisionInfo.contactPointA;
	}
}

void PlayerObject::OnCollisionExit(const CollisionInfo& collisionInfo){
	if (collisionInfo.otherObject->getIsPaintball()) return;
	auto it = std::find(collidedObjects.begin(), collidedObjects.end(), collisionInfo.otherObject);
	if (it != collidedObjects.end()) {
		collidedObjects.erase(it);
		if (collided > 0) {
			collided--;
		}
	}
}

void PlayerObject::OnCollisionStay(const CollisionInfo& collision){
	if (collision.otherObject->getIsPaintball()) return;
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
	else { // not counted as floor yet
		if (angle <= 25.0f) {
			collided++;
			collidedObjects.push_back(collision.otherObject);
		}
	}
	// set special type collision
    collisionType = collision.otherObject->getType();
    if (collision.otherObject->getType() == GameObject::Type::JumpPad || collision.otherObject->getType() == GameObject::Type::Slime) {
		collisionNormal = collision.contactNormal;
		collisionPoint = collision.contactPointA;
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


