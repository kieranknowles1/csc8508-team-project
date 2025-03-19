#include "GameWorld.h"
#include "GameObject.h"
#include "Camera.h"

using namespace NCL;
using namespace NCL::CSC8503;

GameWorld::GameWorld()	{
	worldIDCounter		= 0;
	worldStateCounter	= 0;
}

GameWorld::~GameWorld()	{
	ClearAndErase();
}

void GameWorld::Clear() {
	gameObjects.clear();
	worldIDCounter		= 0;
	worldStateCounter	= 0;
}

void GameWorld::ClearAndErase() {
	for (auto& i : gameObjects) {
		delete i;
	}
	for (auto& i : lights) {
		delete i;
	}
	lights.clear();
	Clear();
}

void GameWorld::AddGameObject(GameObject* o) {
	gameObjects.emplace_back(o);
	o->SetWorldID(worldIDCounter++);
	worldStateCounter++;
}

void GameWorld::RemoveGameObject(GameObject* o) {
	gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), o), gameObjects.end());
	worldStateCounter++;
}

void GameWorld::AddPointLight(PointLight* p) {
	lights.emplace_back(p);
}

void GameWorld::RemovePointLight(PointLight* p) {
	lights.erase(std::remove(lights.begin(), lights.end(), p), lights.end());
}

void GameWorld::GetObjectIterators(
	GameObjectIterator& first,
	GameObjectIterator& last) const {

	first	= gameObjects.begin();
	last	= gameObjects.end();
}

void GameWorld::OperateOnContents(GameObjectFunc f) {
	for (GameObject* g : gameObjects) {
		f(g);
	}
}

void GameWorld::UpdateWorld(float dt) {
	auto rng = std::default_random_engine{};

	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine e(seed);

	for (auto& i : gameObjects) {
		i->Update(dt);
		if (i->getType() == GameObject::Type::Gun) {
            btVector3 origin = i->GetPhysicsObject()->GetRigidBody()->getWorldTransform().getOrigin();
			std::cout << i->GetWorldID() << " Gun at: ";
			std::cout << origin.x() << ", ";
			std::cout << origin.y() << ", ";
			std::cout << origin.z() << "\n";
		}

	}
}
