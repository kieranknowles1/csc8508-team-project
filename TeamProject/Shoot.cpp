#include "Shoot.h"
#include "PointLight.h"
#include "PlayerObject.h"
#include "TutorialGame.h"
#include "Colors.h"
#include "Multiplayer/GamePackets.hpp"
#include "Wanderer.h"

using namespace NCL;
using namespace CSC8503;


std::optional<ShotInfo> Shoot::RayClosest(btVector3 startPos, btVector3 dir, bool hitPlayer) {
	dir.normalize();
    btVector3 shotPos = (startPos + (dir * 10000));
    btCollisionWorld::AllHitsRayResultCallback callback(startPos, shotPos);
    bulletWorld->rayTest(startPos, shotPos, callback);

    btVector3 hitPoint = btVector3();
    btVector3 hitNormal = btVector3(0, 1, 0); // Default normal (up)

    if (callback.hasHit()) {
        GameObject* hitObj = nullptr;
        float smallestDist = INFINITY;
        for (int i = 0; i < callback.m_collisionObjects.size(); i++) { // loop all hits
            GameObject* hit = static_cast<GameObject*>(callback.m_collisionObjects[i]->getUserPointer());
            if (hit != gun && (hitPlayer || hit != player)) {
                // ignore paintballs
                btVector3 posHit = callback.m_hitPointWorld[i];
                float distance = startPos.distance(posHit);
                if (distance < smallestDist) { // find closest valid hit
                    smallestDist = distance;
                    hitPoint = posHit;
                    hitNormal = callback.m_hitNormalWorld[i]; // Get the normal of the hit
                    hitObj = hit;
                }
            }
        }
        return std::make_optional<ShotInfo>(hitObj, hitPoint, hitNormal);
    }
    return std::nullopt;
}

std::optional<ShotInfo> Shoot::ShootBulletPlayer(btVector3 startPos, btVector3 dir, btQuaternion rotation, float dt, int shotID) {
    return RayClosest(startPos, dir);
}

std::optional<ShotInfo> Shoot::ShootBulletAI(btVector3 startPos, btVector3 dir, btQuaternion rotation,float dt) {
    return RayClosest(startPos, dir, true);
}


void Shoot::SpawnDecal(btVector3 hitPos,btVector3 hitNormal, btVector4 color) {
    // Choose a random decal texture
    std::shared_ptr<NCL::Rendering::Texture> pngTexture = decalSystem->PickRandomDecal(decalTextures);

    // Generate a random rotation angle for the decal
    float decalRotation = decalSystem->GetRandomRotation();

    DecalSystem::Decal decal = { hitPos, decalRotation, hitNormal, decalRadius, pngTexture, alphaFade, color };
    decalSystem->ApplyDecal(decal); // Apply the decal using the hit position and normal
}