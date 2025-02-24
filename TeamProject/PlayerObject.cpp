#include "PlayerObject.h"

#include "BWHelperFunctions.h"

GameObject* PlayerObject::getGround(btDiscreteDynamicsWorld* world)
{
    auto shape = (btCapsuleShape*)GetPhysicsObject()->GetRigidBody()->getCollisionShape();
    float rayLength = shape->getRadius() + shape->getHalfHeight() + groundRayOffset;

    BWHelperFunctions helpers(world);
    auto position = GetTransform().getOrigin();
    auto result = helpers.Raycast(position, position + (upDirection * -rayLength));

    return result.second;
}
