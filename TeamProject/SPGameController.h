#pragma once

namespace NCL {
	namespace CSC8503 {
        class TutorialGame;
        class GameObject;
        class NavMesh;
        class Wanderer;
        class Turret;

		class SPGameController {
		public:
			SPGameController(GameObject* p, TutorialGame* g);
            void Update(float dt);
		private:
			GameObject* player;
			TutorialGame* game;

            NavMesh* bottom;
            NavMesh* top;
            NavMesh* front;
            NavMesh* back;
            NavMesh* left;
            NavMesh* right;
            std::vector<NavMesh*> navMeshes;
            bool navMeshDebug = false;
            bool enableAI = true;
            void VisualiseNavMesh();

            std::vector<Wanderer*> wanderers;
            Wanderer* AddWandererToWorld(NavMesh* navMesh, char side, int laserID);

            Turret* AddTurretToWorld();
            Turret* testTurret = nullptr;
		};
	}
}