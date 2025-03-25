#include "AnimationObject.h"

using namespace NCL;
using namespace Rendering;
using namespace CSC8503;

AnimationObject::AnimationObject(GameObject* parent) {
    animations[JUMPING_SPRINT] = new MeshAnimation("/RacerGuy/RunningJump.anm");
    animations[JUMPING_STANDING] = new MeshAnimation("/RacerGuy/WeaponJump.anm");
    animations[SLIDING] = new MeshAnimation("/RacerGuy/WeaponIdle.anm");
    animations[FALLING] = new MeshAnimation("/RacerGuy/FallingIdle.anm");
    animations[SPRINTING_FORWARD] = new MeshAnimation("/RacerGuy/WeaponRun.anm");
    animations[SPRINTING_BACK] = new MeshAnimation("/RacerGuy/WeaponRunBackwards.anm");
    animations[SPRINTING_LEFT] = new MeshAnimation("/RacerGuy/FastStrafeLeft.anm");
    animations[SPRINTING_RIGHT] = new MeshAnimation("/RacerGuy/FastStrafeRight.anm");
    animations[WALKING_FORWARD] = new MeshAnimation("/RacerGuy/WeaponWalk.anm");
    animations[WALKING_BACK] = new MeshAnimation("/RacerGuy/WeaponWalkBackwards.anm");
    animations[WALKING_LEFT] = new MeshAnimation("/RacerGuy/StrafeLeft.anm");
    animations[WALKING_RIGHT] = new MeshAnimation("/RacerGuy/StrafeRight.anm");
    animations[IDLE] = new MeshAnimation("/RacerGuy/WeaponIdle.anm");
    this->parent = parent;
}

AnimationObject::~AnimationObject() {
    for (auto& anim : animations) {
        delete anim.second;
    }
    animations.clear();
}

MeshAnimation* AnimationObject::getAnimation(AnimationState state) {
    auto it = animations.find(state);
    return (it != animations.end()) ? it->second : nullptr;
}
