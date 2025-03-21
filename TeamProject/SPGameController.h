#pragma once

namespace NCL {
	namespace CSC8503 {
        class TutorialGame;
        class GameObject;
        class NavMesh;
        class Wanderer;
        class Turret;
        class GameTechRendererInterface;

        enum class Side {
            BOTTOM,
            TOP,
            FRONT,
            BACK,
            LEFT,
            RIGHT
        };

		class SPGameController {
		public:
			SPGameController(GameObject* p, TutorialGame* g, GameTechRendererInterface* r);
            void Update(float dt);
            void AddIDToPool(int i) { laserIDs.push_back(i); }
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
            Wanderer* AddWandererToWorld(NavMesh* navMesh, Side side);

            Turret* AddTurretToWorld();
            Turret* testTurret = nullptr;

            GameTechRendererInterface* renderer;

            std::vector<int> laserIDs;
            int GetIDFromPool() {
                int id = laserIDs.back();
                laserIDs.pop_back();
                return id;
            }

            int score;
            int level;
            void ClearAIs();
            
		};
	}
}