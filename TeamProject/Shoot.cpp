#include "Shoot.h"

using namespace NCL;
using namespace CSC8503;


ShotInfo* Shoot::RayClosest(btVector3 startPos, btVector3 dir) {
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
        ShotInfo* shotInfo = new ShotInfo(hitObj, hitPoint, hitNormal);
        return shotInfo;
    }
    return nullptr;
}

ShotInfo* Shoot::ShootBulletPlayer(btVector3 startPos, btVector3 dir, btQuaternion rotation) {
    ShotInfo* rayInfo = RayClosest(startPos, dir);
    SpawnBulletMesh(startPos, dir, rotation, rayInfo);
    SpawnDecal(rayInfo);
    return rayInfo;

}

void Shoot::SpawnBulletMesh(btVector3 startPos, btVector3 dir, btQuaternion rotation, ShotInfo* rayInfo) {

    btMatrix3x3 rotationMatrix(rotation);
    btVector3 adjustedOffset = rotationMatrix * bulletCameraOffset;
    btVector3 bulletPos = startPos + adjustedOffset;
    btVector3 shorDirection = (rayInfo->hitPos - bulletPos).normalize();

    Paintball* paintball = new Paintball();
    paintball->Initialise(player);
    Vector3 bulletSize(0.25f, 0.25f, 0.25f);
    paintball->setInitialPosition(bulletPos);
    paintball->setRenderScale(bulletSize);
    paintball->SetRenderObject(new RenderObject(
        paintball,
        resourceManager->getMeshes().get("Sphere.msh"),
        nullptr
    ));
    paintball->GetRenderObject()->SetIsFlat(true);
    paintball->SetPhysicsObject(new PhysicsObject(paintball));
    paintballColor = btVector4(rand() % 2, rand() % 2, rand() % 2, 1);
	Vector4 paintballColorVec4(paintballColor.getX(), paintballColor.getY(), paintballColor.getZ(), paintballColor.getW());
    paintball->GetRenderObject()->SetColour(paintballColorVec4);

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
		int randomIndex = rand() % decalTextures.size();
		std::shared_ptr<NCL::Rendering::Texture> pngTexture = decalTextures[randomIndex];
        decalColor = paintballColor;

        DecalSystem::Decal decal = { shotinfo->hitPos, shotinfo->hitNormal, decalRadius, pngTexture,alphaFade,decalColor };
        decalSystem->ApplyDecal(decal); // Apply the decal using the hit position and normal
    }
}