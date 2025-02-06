#pragma once
#include "Vector.h"
#include "Matrix.h"
#include "Camera.h"
#include "Controller.h"
#include "GameObject.h"

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>


namespace NCL {
	namespace CSC8503 {
		class PlayerController {
		public:
			PlayerController(GameObject* playerIn, const Controller& c, Camera* cam, btDiscreteDynamicsWorld* bulletWorldIn) {
				player = playerIn;
				controller = &c;
				camera = cam;
				bulletWorld = bulletWorldIn;
			}
			~PlayerController();
			void UpdateMovement(float dt);

		private:
			float playerSpeed = 6000.0f;
			float jumpHeight = 4000.0f;
			float maxJumpTime = 0.2f;
			float gravityScale = 1500.0f;
			float strafeMulti = 0.6f;
			float backwardsMulti = 0.55f;
			float sprintMulti = 2.0f;


			bool inAir = false;
			float spaceCount = 0;
			float inAirCount = 0;
			btDiscreteDynamicsWorld* bulletWorld;
			GameObject* player;
			const Controller* controller = nullptr;
			Camera* camera = nullptr;
			float yaw = 0;
		};
	};
}