#pragma once

#include <vector>
#include <Vector.h>
#include <OGLTexture.h>
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

			GLuint GetDecalTexture() const { return decalTexture; }
			GLuint GetDecalFBO() const { return decalFBO; }

			const std::vector<Decal>& GetDecals() const { return decals; }

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