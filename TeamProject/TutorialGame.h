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
#include "Multiplayer/Lobby.hpp"

#include <btBulletDynamicsCommon.h>
#include "SPGameController.h"

namespace NCL {
    namespace CSC8503 {
        class BulletDebug;
        class Config;

        const int MAX_PLAYERS = 8;

        enum class GameMode {
            SINGLEPLAYER,
            HOST_GAME,
            JOIN_GAME,
            CREDITS,
            QUIT,
        };

        enum class GameState {
            IDLE,
            ACTIVE
        };


        class TutorialGame {
        private:
            static TutorialGame* instance;

        public:
            // Physics update frequency, in seconds.
            const static constexpr float PHYSICS_PERIOD = 1.0f / 60.0f;

            static TutorialGame* getInstance() {
                assert(instance && "TutorialGame is not initialised");
                return instance;
            }

            ResourceManager* GetResourceManager() {
                return resourceManager.get();
            }

            static void Start();


            /**
             * @brief Get the Network Instance of the Server.
             * @return
             */
            inline static std::optional<Network>& GetServerInstance() { return server; }

            /**
             * @brief Get the lobby instance.
             */
            inline static std::optional<Lobbies::Lobby>& GetLobby() { return lobby; }

            /**
             * @brief Get the user object for this player.
             */
            inline static std::optional<Lobbies::User>& GetUser() { return user; }

            /**
             * @brief Set the user object for this player.
             */
            inline static void SetUser(Lobbies::User newUser) { user.emplace(newUser); }

            /**
             * @brief Increment user id by 1, generating a new unique ID.
             */
            inline static int GenerateUserID() { USER_ID++; return USER_ID; }

            /**
             * @brief Used to automatically increment the userID from received
             * UserInfoPackets.
             *
             * Helps guarantee unique user ID's if in future we allow users to
             * take control of the server.
             */
            inline static void UpdateUserID(int id) { if (id > USER_ID) USER_ID = id; }


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
            void JoinGame(bool host);
            void ClearWorld();

            GameTechRendererInterface* GetUIRenderer() {
                return renderer;
            }

            GameWorld* getWorld() {
                return world.get();
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

            inline static bool IsHost() { return host; }

            // FIX ME make this protected/private.
            PlayerObject* player;

            void SetGameMode(GameMode gm) { gameMode = gm; }

            SPGameController* GetSPMode() { return spGameController; }

        protected:
            void InitialiseAssets();

            void InitCamera();
            void UpdateKeys();
            void ThirdPersonControls();

            void InitWorld();
            //void ResetWorld();

            void SetupHost() {};

            /**
             * @brief Initialise the network object and run it.
             * @param host Whether the network should be the host server.
             */
            void InitNetwork(bool host = false);

            /**
             * @brief Connect to a host game at the given address.
             *
             * Connection resolution is handled by packet response.
             *
             * @param address ENetAddress of the servers location.
             */
            void ConnectToServer(ENetAddress& address);

            void StartMultiplayerGame();

            void CreateLocal();
            void InitPacketHandlers();
            void ExecuteIncomingPackets();

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
            inline static std::optional<Network> server = std::optional<Network>();
            inline static std::optional<Lobbies::Lobby> lobby = std::optional<Lobbies::Lobby>();
            inline static std::optional<Lobbies::User> user = std::optional<Lobbies::User>();
            inline static int USER_ID = 0;
            inline static bool host = false;

            GameState state = GameState::IDLE;

            GameMode gameMode;
        };
    }
}
