#include "Shoot.h"
#include "PointLight.h"
#include "PlayerObject.h"
#include "TutorialGame.h"
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
            if (!hit->getIsPaintball() && hit != player && hit != gun) { // ignore paintballs
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

std::optional<ShotInfo> Shoot::ShootBulletPlayer(btVector3 startPos, btVector3 dir, btQuaternion rotation, float dt) {
    auto rayInfo = RayClosest(startPos, dir);
    // Don't have Rust style and_then until C++23 :(
    if (rayInfo.has_value()) {
        if (rayInfo.value().hitObj->getType() == GameObject::Type::Player) {
            PlayerObject* hit = (PlayerObject*) rayInfo.value().hitObj;
            hit->Damage(100.0f * dt); // TODO: Don't hard code this.

            if (TutorialGame::GetServerInstance().has_value()) {
                std::shared_ptr<Packet::DamagePacket> damagePacket = std::make_shared<Packet::DamagePacket>(
                    hit->GetWorldID(),
                    100.0f * dt, // TODO: Don't hard code this.
                    TutorialGame::GetUser()->GetUserID()
                );
                TutorialGame::GetServerInstance()->Broadcast(damagePacket);
            }
        }
       // SpawnBulletMesh(startPos, dir, rotation, &rayInfo.value());
        SpawnDecal(&rayInfo.value());
    }
    return rayInfo;

}

void Shoot::SpawnBulletMesh(btVector3 startPos, btVector3 dir, btQuaternion rotation, ShotInfo* rayInfo) {

    btMatrix3x3 rotationMatrix(rotation);
    btVector3 adjustedOffset = rotationMatrix * bulletCameraOffset;
    btVector3 bulletPos = startPos + adjustedOffset;
    btVector3 shorDirection = (rayInfo->hitPos - bulletPos).normalize();

    Paintball* paintball = new Paintball();
    paintball->Initialise(player);
    Vector3 bulletSize(0.5f, 0.5f, 0.5f);
    paintball->setInitialPosition(bulletPos);
    paintball->setRenderScale(bulletSize);
    paintball->SetRenderObject(new RenderObject(
        paintball,
        resourceManager->getMeshes().get("Sphere.msh"),
        nullptr
    ));
    paintball->GetRenderObject()->SetIsFlat(true);
    paintball->SetPhysicsObject(new PhysicsObject(paintball));
    paintballColor = btVector4(rand()%2, rand() % 2 , rand() % 2 ,1);
    paintball->GetRenderObject()->SetColour(paintballColor);

    btCollisionShape* shape = new btSphereShape(1.0f);
    shape->setMargin(0.01f);
    paintball->GetPhysicsObject()->InitBulletPhysics(bulletWorld, shape, 1.0f);
    world->AddGameObject(paintball);

    btVector3 playerVelocity = player->GetPhysicsObject()->GetRigidBody()->getLinearVelocity();
    btVector3 bulletVelocity = playerVelocity + (shorDirection * bulletSpeed);
    // Apply impulse
    paintball->setIsPaintball(true);
    paintball->GetPhysicsObject()->GetRigidBody()->setGravity(btVector3(0, 0, 0));
    paintball->GetPhysicsObject()->GetRigidBody()->applyCentralImpulse(bulletVelocity);
    paintball->GetPhysicsObject()->GetRigidBody()->activate();
}

void Shoot::SpawnDecal(ShotInfo* shotinfo) {
    if (shotinfo->hitObj != nullptr) {
		// Choose a random decal texture
        std::shared_ptr<NCL::Rendering::Texture> pngTexture = decalSystem->PickRandomDecal(decalTextures);

		// Use the same color for the decal as the paintball
        btVector4 decalColor = paintballColor;

		// Generate a random rotation angle for the decal
        float decalRotation = decalSystem->GetRandomRotation();

        DecalSystem::Decal decal = { shotinfo->hitPos, decalRotation, shotinfo->hitNormal, decalRadius, pngTexture,alphaFade,decalColor };
        decalSystem->ApplyDecal(decal); // Apply the decal using the hit position and normal
    }
}