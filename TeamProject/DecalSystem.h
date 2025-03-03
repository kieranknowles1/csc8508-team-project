#pragma once

#include <vector>
#include <Vector.h>
#include "Maths.h"
#include "Texture.h"

using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {

		class DecalSystem {
		public:
			struct Decal {
				Vector3 position;
				Vector3 normal;
				float radius;
				std::shared_ptr<Rendering::Texture> texture;
				float alphaFade;
			};

			DecalSystem(int width, int height);
			~DecalSystem();

			void ApplyDecal(Decal& decal);
			void Update(float dt);

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
		};
	}
}