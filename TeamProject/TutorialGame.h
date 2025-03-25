#pragma once

#include "../NCLCoreClasses/KeyboardMouseController.h"

#include "GameTechRendererInterface.h"
#include "ResourceManager.h"

#include "LevelImporter.h"
#include "PlayerController.h"
#include "PlayerObject.h"
#include "Turret.h"
#include "NavMesh.h"
#include "Profiler.h"
#include "Wanderer.h"
#include "Respawn.h"
#include "Network/Network.hpp"
//#include "Multiplayer/Server.hpp"

#include <shared_mutex>
#include <btBulletDynamicsCommon.h>
#include "SPGameController.h"

namespace Multiplayer {
    class Server;
}

namespace NCL {
    namespace CSC8503 {
        class BulletDebug;
        class Config;

        const int MAX_PLAYERS = 8;
        // Max duration to wait for a connection when joining, in milliseconds
        const static constexpr unsigned int ConnectionTimeout = 1000;

        enum class GameMode {
            SINGLEPLAYER,
            HOST_GAME,
            JOIN_GAME,
            CREDITS,
            QUIT,
        };

        enum class GameState {
            IDLE,
            STARTING,
            ACTIVE
        };


        class TutorialGame {
        private:
            static TutorialGame* instance;

        public:
            // Physics update frequency, in seconds.
            const static constexpr float PHYSICS_PERIOD = 1.0f / 60.0f;

            static TutorialGame* getInstance() {
                //assert(instance && "TutorialGame is not initialised");
                return instance;
            }

            ResourceManager* GetResourceManager() {
                return resourceManager.get();
            }

            static void Start();


            /**
             * @brief Get the Network Instance of the Server.
             */
            Multiplayer::Server* GetServerInstance() { return server; }

            GameState GetState() const { 
                std::shared_lock lock(*stateMutex);
                return state;
            }

            void SetState(GameState newState) {
                std::unique_lock lock(*stateMutex);
                state = newState;
            }

            // Remove an object at the end of this frame. Use during update to avoid removing
            // from containers while iterating
            // It is the caller's responsibility to ensure there are no dangling references from other objects
            //
            // No-op if called multiple times on the same object
            //
            // Deletion process:
            // - delayedRemoveObject - adds to graveyard and sets `deleted` flag
            // - deleted flag - suppresses render & updates, instructs objects to remove their references
            // - clearGraveyard - after 2 frames in the graveyard, to ensure everything has had at least 1 update call, delete the object for good
            void delayedRemoveObject(GameObject* obj) {
                if (obj->isDeleted()) return;
                obj->setDeleted();
                earlyGraveyard.push_back(obj);
            }

            TutorialGame(GameTechRendererInterface* renderer, Controller* controller, Config& config);
            ~TutorialGame();

            virtual void UpdateGame(float dt);
            void LoadWorldFromFile(int levelNum);

            bool StartMultiplayerGame(bool isHost);
            void ClearWorld();

            GameWorld* GetWorld() const { return world.get(); }

            GameTechRendererInterface* GetUIRenderer() {
                return renderer;
            }

            btDiscreteDynamicsWorld* getBulletWorld() {
                return bulletWorld;
            }

            ResourceManager* getResourceManager() {
                return resourceManager.get();
            }

            std::shared_ptr<Texture> getDefaultTexture() {
                return defaultTexture;
            }

            // FIX ME make this protected/private.
            PlayerObject* player;

            void SetGameMode(GameMode gm) { gameMode = gm; }

            SPGameController* GetSPMode() { return spGameController; }

            void SetFreeCam(bool b) { freeCam = b; }

        protected:
            void InitialiseAssets();

            void InitCamera();
            void UpdateKeys();
            void ThirdPersonControls();

            void InitWorld();
            //void ResetWorld();
            void UpdatePlayer(float dt);

            GameObject* AddFloorToWorld(const Vector3& position, const Vector3& size, const Vector3& rotation);
            GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
            GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f,bool hasCollision = true);
            PlayerObject* AddPlayerCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass = 10.0f);
            GameObject* AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass = 10.0f);

            GameObject* AddInfinitePlaneToWorld(const Vector3& position, const Vector3& normal, float planeConstant);

            std::unique_ptr<ResourceManager> resourceManager;
            bool showProfiling = false;
            Profiler profiler;

            GameTechRendererInterface* renderer;
            std::unique_ptr<GameWorld> world;
            Config& config;

            std::vector<GameObject*> earlyGraveyard; // Added this frame
            std::vector<GameObject*> lateGraveyard; // Added previous frame
            void clearGraveyard();

            Controller* controller;

            std::shared_ptr<Texture> defaultTexture;
            std::shared_ptr<Texture> paintballTexture;

            //Coursework Additional functionality
            GameObject* lockedObject	= nullptr;
            Vector3 lockedOffset		= Vector3(0, 14, 20);
            void LockCameraToObject(GameObject* o) {
                lockedObject = o;
            }

            GameObject* objClosest = nullptr;

            /* bullet physics stuff here */
            btDiscreteDynamicsWorld* bulletWorld = nullptr;
            btBroadphaseInterface* broadphase = nullptr;
            btDefaultCollisionConfiguration* collisionConfig = nullptr;
            btCollisionDispatcher* dispatcher = nullptr;
            btSequentialImpulseConstraintSolver* solver = nullptr;

            BulletDebug* bulletDebug = nullptr;

            void CheckCollisions();
            void DestroyBullet();
            void InitBullet(); // Initialises the Bullet physics world

            //Player things
            PlayerObject* InitPlayer(btVector3 position, btVector3 upDir);
            PerspectiveCamera* mainCamera;
            std::unique_ptr<PlayerController> playerController;
            bool freeCam = false;
            bool thirdPerson = false;
            Vector4 playerColour = Vector4(1, 0.8, 1, 1);

            //fixed update
            float accumulator = 0.0f;
            float fixedDeltaTime = 1.0f / 60.0f;

            //Level import
            bool loadFromLevel;

            SPGameController* spGameController = nullptr;

            GameObject* AddGunToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, bool hasCollision);

            //post processing time variable effects
            float pulse = 0;

            //to allow mesh animation to access time:
            int aniCurrentFrame;
            float aniFrameTime;

        private:
            Multiplayer::Server* server = nullptr;
            std::shared_mutex* stateMutex = nullptr;
            GameState state = GameState::IDLE;

            GameMode gameMode;
        };
    }
}
