#include "Shoot.h"
#include "PointLight.h"
#include "PlayerObject.h"
#include "TutorialGame.h"
#include "Colors.h"
#include "Multiplayer/GamePackets.hpp"

using namespace NCL;
using namespace CSC8503;


std::optional<ShotInfo> Shoot::RayClosest(btVector3 startPos, btVector3 dir, GameObject * ignore) {
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
            if (hit != player && hit != gun) { // ignore paintballs
                if (ignore) {
                    if (hit == ignore) continue;
                }
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
    auto rayInfo = RayClosest(startPos, dir);
    // Don't have Rust style and_then until C++23 :(
    if (rayInfo.has_value()) {
        if (rayInfo.value().hitObj->getType() == GameObject::Type::Player) {
            PlayerObject* hit = (PlayerObject*) rayInfo.value().hitObj;
            hit->Damage(100.0f * dt); // TODO: Don't hard code this.

            if (TutorialGame::GetServerInstance().has_value()) {
                std::shared_ptr<Packet::DamagePacket> damagePacket = std::make_shared<Packet::DamagePacket>(
                    hit->GetWorldID(),
                    200.0f * dt, // TODO: Don't hard code this.
                    TutorialGame::GetUser()->GetUserID()
                );
                TutorialGame::GetServerInstance()->Broadcast(damagePacket);
            }
        }
       // SpawnBulletMesh(startPos, dir, rotation, &rayInfo.value());
        SpawnDecal(rayInfo.value().hitPos, rayInfo.value().hitNormal, shotID);
    }
    return rayInfo;
}

std::optional<ShotInfo> Shoot::ShootBulletAI(btVector3 startPos, btVector3 dir, btQuaternion rotation,float dt) {
    auto rayInfo = RayClosest(startPos, dir);
    // Don't have Rust style and_then until C++23 :(
    if (rayInfo.has_value()) {
        if (rayInfo.value().hitObj->getType() == GameObject::Type::Player) {
            PlayerObject* hit = (PlayerObject*)rayInfo.value().hitObj;
            hit->Damage(100.0f * dt); // TODO: Don't hard code this.
        }
        SpawnDecal(rayInfo.value().hitPos, rayInfo.value().hitNormal, 1);
    }
    return rayInfo;
}


void Shoot::SpawnDecal(btVector3 hitPos,btVector3 hitNormal, int shotID) {

		// Choose a random decal texture
        std::shared_ptr<NCL::Rendering::Texture> pngTexture = decalSystem->PickRandomDecal(decalTextures);

		// Use the same color for the decal as the paintball
        btVector4 decalColor = Color::GetPlayerColor(shotID);

		// Generate a random rotation angle for the decal
        float decalRotation = decalSystem->GetRandomRotation();

        DecalSystem::Decal decal = { hitPos, decalRotation, hitNormal, decalRadius, pngTexture,alphaFade,decalColor };
        decalSystem->ApplyDecal(decal); // Apply the decal using the hit position and normal
    
}