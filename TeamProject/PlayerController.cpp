#include "PlayerController.h"
#include "AudioEngine.h"
#include "TutorialGame.h"

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
    pngTexture = resourceManager->getTextures().get(decalTexturePath);
}

btVector3 GetEulerAngles(btQuaternion quat) {
    btScalar roll2, pitch2, yaw2;
    quat.getEulerZYX(yaw2, pitch2, roll2);
    return btVector3(pitch2, roll2, yaw2) * (180.0f / SIMD_PI);
}


void PlayerController::UpdateMovement(float dt) {
    transformPlayer = rb->getWorldTransform();
    btPlayerPos = transformPlayer.getOrigin();

    HandleShooting(dt);

    // yaw is fully local
    yaw = fmod(yaw - controller->GetAnalogue(Controller::AnalogueControl::LookX) + 360.0f, 360.0f);
    camera->SetYaw(yaw);

    //camera offset for rotation
    btVector3 rot = GetEulerAngles(camRotOffset);
    camera->setRotation(rot);

    HandleTypes();

    //sliding/floor detection
    HandleSliding(dt);
    HandleCrouching(dt);

    if (inAirTime > 0) {
        player->setCollided(0);
        inAirTime -= dt;
    }

    if ((isSliding||slideTransition) && !isCrouching) return;

    //player rotation
    btQuaternion playerYaw = btQuaternion(btVector3(0, 1, 0), Maths::DegreesToRadians(yaw));
    btQuaternion finalRotation = camRotOffset * playerYaw;
    transformPlayer.setRotation(finalRotation);
    rb->setWorldTransform(transformPlayer);

    //camera follows player, lowers if crouching
    btTransform transformPlayerMotion;
    player->GetPhysicsObject()->GetMotionState()->getWorldTransform(transformPlayerMotion);
    btVector3 playerCamPos = transformPlayerMotion.getOrigin();
    playerCamPos += upDirection * (isCrouching ? std::lerp(cameraHeight, crouchHeight, btMin(currentCrouchingTimer / crouchingTime, 1.0f)) : std::lerp(crouchHeight, cameraHeight, btMin(currentStandingTimer / crouchingTime, 1.0f)));
    if (!slideTransition && !thirdPerson) {
        camera->SetPosition(playerCamPos);
        SetGunTransform();
    }

    //finds player forward and right vectors
    btMatrix3x3 rotationMatrix(finalRotation);
    btVector3 forward = rotationMatrix * btVector3(0, 0, -1);
    btVector3 up = rotationMatrix * btVector3(0, 1, 0);
    btVector3 right = rotationMatrix * btVector3(1, 0, 0);

    //if on ground, movement based on floor angle
    if (player->getCollided() > 0) {
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

    //movement based on all the multipliers combined
    Vector2 directionalInput = getDirectionalInput();
    bool sprinting = controller->GetDigital(Controller::DigitalControl::Sprint);
    float forwardMovement = directionalInput.y;
    float moveMulti = playerSpeed * (sprinting ? sprintMulti : 1) * (isCrouching ? crouchMulti : 1) * (player->getCollided() <= 0 ? airMulti : 1);
    forwardMovement *= (forwardMovement <= 0) ? backwardsMulti : 1;
    btVector3 movement = (right * directionalInput.x * strafeMulti * moveMulti ) +(forward * forwardMovement * moveMulti);

    if (player->getCollided() <= 0 || onIce) {
        movement *= (airMulti*dt);
        movement += rb->getLinearVelocity();
    }

    // jump input
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
        player->setCollided(0);
        inAirTime = 0.2f;
    }
    previousVelocity = rb->getLinearVelocity();
    rb->setLinearVelocity(movement);
    rb->activate();

}


//attaches gun to the camera position/rotation
void PlayerController::SetGunTransform() {
    float pitchRadians = Maths::DegreesToRadians(camera->GetPitch());
    float yawRadians = Maths::DegreesToRadians(camera->GetYaw());
    btQuaternion yawQuat(btVector3(0, 1, 0), yawRadians);
    btQuaternion pitchQuat(btVector3(1, 0, 0), pitchRadians);
    btQuaternion gunRotation = camRotOffset * yawQuat * pitchQuat; // Yaw first, then pitch

    btMatrix3x3 rotationMatrixCam(gunRotation);
    btVector3 adjustedOffset = rotationMatrixCam * gunCameraOffset; // Apply rotation to the offset

    transformGun = gun->GetPhysicsObject()->GetRigidBody()->getWorldTransform();
    btGunPos = camera->GetPosition() + adjustedOffset; // Offset from camera position
    transformGun.setOrigin(btGunPos);
    transformGun.setRotation(gunRotation);

    gun->GetPhysicsObject()->GetRigidBody()->setWorldTransform(transformGun);
}

void PlayerController::HandleShooting(float dt) {
    if (controller->GetDigital(Controller::DigitalControl::Fire) && shotTimer >= shotCooldown) {
        Shoot();
        shotTimer = 0.0f;
    }
    else {
        shotTimer += dt;
    }
}



void PlayerController::Shoot() {
    // Convert camera pitch & yaw to radians
    float pitchRadians = Maths::DegreesToRadians(camera->GetPitch());
    float yawRadians = Maths::DegreesToRadians(yaw);
    btQuaternion yawQuat(btVector3(0, 1, 0), yawRadians);
    btQuaternion pitchQuat(btVector3(1, 0, 0), pitchRadians);
    btQuaternion bulletRotation = camRotOffset * yawQuat * pitchQuat;
    btMatrix3x3 rotationMatrix(bulletRotation);

    btVector3 forwardDir = rotationMatrix * btVector3(0, 0, -1);
    btVector3 forwardPos = (camera->GetPosition() + (forwardDir * 10000));
    btCollisionWorld::AllHitsRayResultCallback callback(camera->GetPosition(), forwardPos);
    bulletWorld->rayTest(camera->GetPosition(), forwardPos, callback);
    
    btVector3 hitPoint = btVector3();
	btVector3 hitNormal = btVector3(0, 1, 0); // Default normal (up)

    if (callback.hasHit()) {
        GameObject* hitObj = nullptr;
        float smallestDist = INFINITY;
        for (int i = 0; i < callback.m_collisionObjects.size(); i++) { // loop all hits
            GameObject* hit = static_cast<GameObject*>(callback.m_collisionObjects[i]->getUserPointer());
            if (!hit->getIsPaintball() && hit != player && hit != gun) { // ignore paintballs
                btVector3 posHit = callback.m_hitPointWorld[i];
                float distance = btPlayerPos.distance(posHit);
                if (distance < smallestDist){ // find closest valid hit
                    smallestDist = distance;
                    hitPoint = posHit;
					hitNormal = callback.m_hitNormalWorld[i]; // Get the normal of the hit
                    hitObj = hit;
                }
            }
        }

        if (hitObj != nullptr) { // hit an object of some kind
         	DecalSystem::Decal decal = { hitPoint, hitNormal, decalRadius, pngTexture,alphaFade,decalColor };
            decalSystem->ApplyDecal(decal); // Apply the decal using the hit position and normal
        }
    }
    ShootBullet(bulletRotation, hitPoint);
}

void PlayerController::ShootBullet(btQuaternion bulletRotation ,btVector3 hitPoint) {

    btMatrix3x3 rotationMatrix(bulletRotation);
    btVector3 adjustedOffset = rotationMatrix * bulletCameraOffset;
    btVector3 bulletPos = camera->GetPosition() + adjustedOffset;
    btVector3 shorDirection = (hitPoint - bulletPos).normalize();

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
    paintball->GetRenderObject()->SetColour(Vector4(rand() % 2, rand() % 2, rand() % 2, 1));

    btCollisionShape* shape = new btSphereShape(1.0f);
    shape->setMargin(0.01f);
    paintball->GetPhysicsObject()->InitBulletPhysics(bulletWorld, shape, 1.0f);
    world->AddGameObject(paintball);
    btVector3 playerVelocity = rb->getLinearVelocity();
    btVector3 bulletVelocity = playerVelocity + (shorDirection * bulletSpeed);

    // Apply impulse
    paintball->setIsPaintball(true);
    paintball->GetPhysicsObject()->GetRigidBody()->setGravity(btVector3(0, 0, 0));
    paintball->GetPhysicsObject()->GetRigidBody()->applyCentralImpulse(bulletVelocity);
    paintball->GetPhysicsObject()->GetRigidBody()->activate();
}


//transitions states between standing and crouching
void PlayerController::HandleCrouching(float dt) {
    if (isSliding) {
        return;
    }
    if ((crouching && !controller->GetDigital(Controller::DigitalControl::Crouch)) || slideTransition) {
        crouching = CheckCeling();
    }
    else {
        crouching = controller->GetDigital(Controller::DigitalControl::Crouch);
    }

    crouchTransition = crouching ? (currentCrouchingTimer < crouchingTime) : (currentStandingTimer < crouchingTime);

    if (crouching) {
        isCrouching = true;
        currentStandingTimer = 0;
        currentCrouchingTimer = btMin(currentCrouchingTimer + dt, crouchingTime);
        currentHeight = std::lerp(standingHeight, crouchingHeight, currentCrouchingTimer / crouchingTime);
    }
    else {
        isCrouching = false;
        currentCrouchingTimer = 0;
        currentStandingTimer = btMin(currentStandingTimer + dt, crouchingTime);
        currentHeight = std::lerp(crouchingHeight, standingHeight, currentStandingTimer / crouchingTime);
    }

    if (crouchTransition) {
        Vector3 currentScale = player->getRenderScale();
        currentScale.y = (currentHeight+2)*0.7;
        player->setRenderScale(currentScale);

        btCollisionShape* shape = player->GetPhysicsObject()->GetRigidBody()->getCollisionShape();
        shape->setLocalScaling(btVector3(1, currentHeight/standingHeight, 1));
    }
}


//uses ray to detect if the player is blocked from standing
bool PlayerController::CheckCeling() {
    btVector3 btBelowPlayerPos = btPlayerPos;
    btBelowPlayerPos += (upDirection * 4.1f);
    btCollisionWorld::ClosestRayResultCallback callback(btPlayerPos, btBelowPlayerPos);
    bulletWorld->rayTest(btPlayerPos, btBelowPlayerPos, callback);
    if (callback.hasHit()) {
        return true;
    }
    else {
        return false;
    }
}

// finds surface normal of floor below
btVector3 PlayerController::FindFloorNormal() {
    btVector3 btBelowPlayerPos = btPlayerPos;
    btBelowPlayerPos -= (upDirection * 16);
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
    bool slidingCondition = crouching && sprinting && !isCrouching;

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

    if ((slideTransition || isSliding) && !isCrouching) {
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
        playerPos += upDirection * std::lerp(isSliding ? cameraHeight : slidingCameraHeight, isSliding ? slidingCameraHeight : cameraHeight, slideFactor);
        if (!thirdPerson) {
            camera->SetPosition(playerPos);
            SetGunTransform();
        }
        //CheckFloor(dt);
        btVector3 pastMovement = rb->getLinearVelocity();
        pastMovement += upDirection * -(gravityScale * dt);
        previousVelocity = rb->getLinearVelocity();
        rb->setLinearVelocity(pastMovement);
        rb->activate();
    }
}

void PlayerController::HandleTypes() {
    switch (player->getType())
    {
    case 'D': //Default
        onIce = false;
        break;
    case 'J': {//Jump-pads
        btVector3 normal = player->getCollisionNormal();
        float dotProduct = normal.dot(upDirection.absolute());
        btVector3 movement = btVector3(0, 0, 0);
        movement += (bouncePadHeight * -player->getCollisionNormal());
        rb->setLinearVelocity(btVector3(0, 0, 0));
        player->setCollided(0);
        inAirTime = 0.2f;
        rb->applyCentralImpulse(movement);
        break;
    }
    case 'S': { // Slime
        if (inAirTime <= 0) {
            btVector3 normal = player->getCollisionNormal();
            float dampening = 0.85f;
            btVector3 reflectedVelocity = previousVelocity - (2 * previousVelocity.dot(normal) * normal);
            if (fabs(10 * previousVelocity.dot(normal)) <= 0.25f) {
                btVector3 direction = (player->getCollisionPoint() - transformPlayer.getOrigin()).normalized();
                float dot = direction.dot(-upDirection);
                float angle = acos(dot) * (180.0f / SIMD_PI);
                if (angle <= 25.0f) { // come to rest on floor
                    player->setCollided(1);
                    break;
                }
            }
            else {
                reflectedVelocity *= dampening;
                inAirTime = 0.1f;
                rb->setLinearVelocity(reflectedVelocity);
            }
        }
        break;
    }
    case 'I': {// Ice
        onIce = true;
        break;
    }
    default:
        break;
    }
    player->resetType();
}


// world rotate things
void PlayerController::Rotate(bool positive, bool rolling) {
    if (rotationChanging) return;
    btVector3 rightDirections = (rolling ? CalculateForwardFromYaw() : CalculateRightFromYaw());
    btQuaternion pitchQuat(rightDirections, Maths::DegreesToRadians(positive ? 90 : -90));
    targetWorldRotation = quatRotate(pitchQuat, upDirection);
    targetcamRotOffset = pitchQuat * oldcamRotOffset; // Apply rotation correctly
    rotationChanging = true;
}

void PlayerController::CalculateDirections(float dt) {
    upDirection = CalculateUpDirection(dt);
    rightDirection = CalculateRightDirection(upDirection);
    forwardDirection = CalculateForwardDirection(upDirection, rightDirection);
    player->setUpDirection(upDirection);

    //Update FMod listener each frame so audio is correctly positioned
    audioEngine.Set3dListenerAndOrientation(camera->GetPosition(), forwardDirection, upDirection);
}

btVector3 PlayerController::CalculateUpDirection(float dt) {

    btVector3 upDir;
    if (!rotationChanging) {
        upDir = targetWorldRotation;
        camRotOffset = targetcamRotOffset;

    }else if (rotateTimer <= rotateTime && rotationChanging) {
        rotateTimer += dt;
        upDir = lerp(oldWorldRotation, targetWorldRotation,  rotateTimer/rotateTime);
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

btVector3 PlayerController::CalculateRightDirection(btVector3 upDir) {
    btVector3 forward = btVector3(0, 0, 1);
    if (fabs(upDir.dot(forward)) > 0.999f) {
        forward = btVector3(0, 1, 0);
    }
    btVector3 rightDirection = upDir.cross(forward);
    rightDirection.normalize();
  //  std::cout << rightDirection << std::endl;
    return rightDirection;
}

btVector3 PlayerController::CalculateForwardDirection(btVector3 upDir,btVector3 rightDir) {
    btVector3 forwardDirection = rightDir.cross(upDir);
    forwardDirection.normalize();
    return forwardDirection;
}

btVector3 PlayerController::CalculateForwardFromYaw() {
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

btVector3 PlayerController::CalculateRightFromYaw() {
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

Vector2 PlayerController::getDirectionalInput() const
{
    Vector2 raw(controller->GetAnalogue(Controller::AnalogueControl::MoveSidestep), controller->GetAnalogue(Controller::AnalogueControl::MoveForward));
    float magnitude = Vector::Length(raw);
    return magnitude <= 1.0f ? raw : raw / magnitude;
}

void NCL::Paintball::OnCollisionEnter(const CollisionInfo &collisionInfo)
{
    if (collisionInfo.otherObject == player) return;

    TutorialGame::getInstance()->delayedRemoveObject(this);
}
