#pragma once

#include <vector>
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

			GLuint GetDecalTexture() const { return decalTexture; }
			GLuint GetDecalFBO() const { return decalFBO; }

			const std::vector<Decal>& GetDecals() const { return decals; }

			void ClearDecalsFromWorld() { decals.clear(); }

		private:
			GLuint decalFBO;
			GLuint decalTexture;

			int textureWidth;
			int textureHeight;

			float decayRate;

			std::vector<Decal> decals;
		};
	}
}