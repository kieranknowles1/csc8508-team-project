#pragma once

#include <vector>
#include <random>

#include <Vector.h>
#include "Maths.h"
#include "Texture.h"
#include "btBulletDynamicsCommon.h"

namespace NCL {
	namespace CSC8503 {

		class DecalSystem {
		public:
			struct Decal {
				btVector3 position;
				float rotation;
				btVector3 normal;
				float radius;
				std::shared_ptr<Rendering::Texture> texture;
				float alphaFade;
				btVector4 color;
			};

			DecalSystem(int width, int height);
			~DecalSystem();

			void ApplyDecal(Decal& decal);
			void Update(float dt);
			float GetRandomRotation();
            std::shared_ptr<NCL::Rendering::Texture> PickRandomDecal(const std::vector<std::shared_ptr<NCL::Rendering::Texture>>& decalsArr);

			unsigned int GetDecalTexture() const { return decalTexture; }
			unsigned int GetDecalFBO() const { return decalFBO; }
			const std::vector<Decal>& GetDecals() const { return decals; }

			void ClearDecalsFromWorld() { decals.clear(); }

		private:
			unsigned int decalFBO;
			unsigned int decalTexture;

			int textureWidth;
			int textureHeight;

			float decayRate;

			std::vector<Decal> decals;

            std::mt19937 gen;
            std::uniform_real_distribution<float> angleDis;
            std::uniform_int_distribution<int> indexDist;
		};
	}
}