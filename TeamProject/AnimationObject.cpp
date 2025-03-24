#include "AnimationObject.h"

using namespace NCL::CSC8503;
using namespace NCL;

AnimationObject::AnimationObject(GameObject* parent) { //maybe doesn't need to be at all linked to gameObject
	this->walk          = new MeshAnimation("/RacerGuy/WeaponWalk.anm"); 
	this->run           = new MeshAnimation("/RacerGuy/WeaponRun.anm");
	this->walkBackwards = new MeshAnimation("/RacerGuy/WeaponWalkBackwards.anm");
	this->runBackwards  = new MeshAnimation("/RacerGuy/WeaponRunBackwards.anm");
	this->strafeRight   = new MeshAnimation("/RacerGuy/StrafeRight.anm");
	this->strafeLeft    = new MeshAnimation("/RacerGuy/StrafeLeft.anm");
	this->standingJump  = new MeshAnimation("/RacerGuy/WeaponJump.anm");
	this->runningJump   = new MeshAnimation("/RacerGuy/RunningJump.anm");
	this->idle          = new MeshAnimation("/RacerGuy/WeaponIdle.anm");
	this->falling       = new MeshAnimation("/RacerGuy/FallingIdle.anm");
	this->sliding       = new MeshAnimation("/RacerGuy/AIOSlideNoRM.anm");

	this->parent = parent;
}

AnimationObject::~AnimationObject() {
	delete walk;
	delete run;
	delete walkBackwards;
	delete runBackwards;
	delete strafeRight;
	delete strafeLeft;
	delete standingJump;
	delete runningJump;
	delete idle;
	delete falling;
	delete sliding;
}