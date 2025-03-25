#pragma once

namespace NCL {
	namespace CSC8503 {
        class TutorialGame;
        class GameObject;
        class NavMesh;
        class Wanderer;
        class Turret;
        class GameTechRendererInterface;
        class PlayerObject;

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
			SPGameController(PlayerObject* p, TutorialGame* g, GameTechRendererInterface* r);
            void Update(float dt);
            void AddIDToPool(int i) { laserIDs.push_back(i); }
		private:
			PlayerObject* player;
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
            int defeated;
            int mult = 1;
            float multTimer = 5.0f;
            float maxMultTimer = 5.0f;

            void InitLevel(int curLevel);
            
		};
	}
}