#pragma once
#include "Texture.h"
#include "Mesh.h"
#include "Buffer.h"
#include "MeshAnimation.h"
#include "RenderObject.h"

namespace NCL {
	using namespace NCL::Rendering;

	namespace CSC8503 {
		class GameObject;  
		using namespace Maths;

		//This class basically just loads all the animations

		class AnimationObject { //No Longer Inheriting from RenderObject. //REMEMBER TO CHANGE FILE NAME!!!!
		public:
			AnimationObject(GameObject* parent); //Maybe include mesh?
			~AnimationObject();

			MeshAnimation* GetWalk() const {
				return walk;
			}

			MeshAnimation* GetRun() const {
				return run; 
			}

			MeshAnimation* GetWalkBack() const {
				return walkBackwards;
			}

			MeshAnimation* GetRunBack() const {
				return runBackwards;
			}

			MeshAnimation* GetStrafeRight() const {
				return strafeRight;
			}

			MeshAnimation* GetStrafeLeft() const {
				return strafeLeft;
			}

			MeshAnimation* GetStandingJump() const {
				return standingJump;
			}

			MeshAnimation* GetRunningJump() const {
				return runningJump;
			}

			MeshAnimation* GetIdle() const {
				return idle;
			}

			MeshAnimation* GetFalling() const {
				return falling;
			}

			MeshAnimation* GetSliding() const {
				return sliding;
			}


		protected:
		//will have a mesh and material instead of singular texture
			MeshAnimation* walk;
			MeshAnimation* run;
			MeshAnimation* walkBackwards;
			MeshAnimation* runBackwards;
			MeshAnimation* strafeRight;
			MeshAnimation* strafeLeft;
			MeshAnimation* standingJump;
			MeshAnimation* runningJump;
			MeshAnimation* idle;
			MeshAnimation* falling;
			MeshAnimation* sliding;

			GameObject* parent;

		};
	}
}