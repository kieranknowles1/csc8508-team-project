#pragma once
#include <random>

#include <Camera.h>
namespace NCL {
		class Camera;
	namespace CSC8503 {
		class GameObject;

		typedef std::function<void(GameObject*)> GameObjectFunc;
		typedef std::vector<GameObject*>::const_iterator GameObjectIterator;

		class GameWorld	{
		public:
			GameWorld();
			~GameWorld();

			void Clear();
			void ClearAndErase();

			void AddGameObject(GameObject* o);
			void RemoveGameObject(GameObject* o, bool andDelete = false);

			PerspectiveCamera& GetMainCamera()  {
				return mainCamera;
			}

			virtual void UpdateWorld(float dt);

			void OperateOnContents(GameObjectFunc f);

			void GetObjectIterators(
				GameObjectIterator& first,
				GameObjectIterator& last) const;

			int GetWorldStateID() const {
				return worldStateCounter;
			}

		protected:
			std::vector<GameObject*> gameObjects;

			PerspectiveCamera mainCamera;

			int		worldIDCounter;
			int		worldStateCounter;
		};
	}
}

