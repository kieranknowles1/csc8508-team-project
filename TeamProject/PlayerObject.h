#pragma once
#include "Vector.h"
#include "Matrix.h"
#include "Camera.h"
#include "Controller.h"
#include "GameObject.h"
#include "PhysicsObject.h"


#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>

using namespace NCL::CSC8503;

// Player class derived from GameObject
class PlayerObject : public GameObject {
public:
    void OnCollisionEnter(GameObject* otherObject, const btVector3& contactPointA, const btVector3& contactPointB) override {
        btVector3 playerPos = this->GetPhysicsObject()->GetRigidBody()->getWorldTransform().getOrigin();
        btVector3 direction = (contactPointA - playerPos).normalize();
        float dotProduct = direction.dot(upDirection);
        if (dotProduct < 0.0f) {
            collidedObjects.push_back(otherObject);
            collided++;
        }
    }

    void OnCollisionExit(GameObject* otherObject) override {
        auto it = std::find(collidedObjects.begin(), collidedObjects.end(), otherObject);
        if (it != collidedObjects.end()) {
            collidedObjects.erase(it);
            if (collided > 0) {
                collided--;
            }
        }
    }

    void setCollided(int collidedIn) {
        collided = collidedIn;
    }

    int getCollided() const {
        return collided;
    }

    void setUpDirection(const btVector3& upDirectionIn) {
        upDirection = upDirectionIn;
    }

private:
    int collided = 0;
    btVector3 upDirection;
    std::vector<GameObject*> collidedObjects; // Using vector instead of list for easier search
};
