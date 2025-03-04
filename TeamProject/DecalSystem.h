#pragma once

#include <vector>

#include <Vector.h>
#include "Maths.h"
#include <OGLTexture.h>
#include "Texture.h"
#include "btBulletDynamicsCommon.h"

namespace NCL {
	namespace CSC8503 {

		class DecalSystem {
		public:
			struct Decal {
				btVector3 position;
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

			unsigned int GetDecalTexture() const { return decalTexture; }
			unsigned int GetDecalFBO() const { return decalFBO; }
			unsigned int GetDecalDepthTexture() const { return decalDepthTexture; }

			const std::vector<Decal>& GetDecals() const { return decals; }

			void ClearDecalsFromWorld() { decals.clear(); }

		private:
			unsigned int decalFBO;
			unsigned int decalTexture;
			unsigned int decalDepthTexture;

			int textureWidth;
			int textureHeight;

			float decayRate;

			std::vector<Decal> decals;
		};
	}
}