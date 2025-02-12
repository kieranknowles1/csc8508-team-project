#pragma once
#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"

namespace NCL {
	using namespace NCL::Rendering;

	namespace CSC8503 {
		class GameObject;
		using namespace Maths;

		class RenderObject
		{
		public:
			RenderObject(GameObject* parent, std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture> tex, std::shared_ptr<Shader> shader, Texture* normal = nullptr);
			~RenderObject();

			void SetDefaultTexture(std::shared_ptr<Texture> t) {
				texture = t;
			}

			Texture* GetDefaultTexture() const {
				return texture.get();
			}

			Mesh* GetMesh() const {
				return mesh.get();
			}

			Shader*	GetShader() const {
				return shader.get();
			}

			void SetColour(const Vector4& c) {
				colour = c;
			}

			Vector4 GetColour() const {
				return  colour;
			}

			GameObject* getParent() const {
				return parent;
			}

			bool GetIsFlat() const{
				return isFlat;
			}

			void SetIsFlat(bool isFlatIn) {
				isFlat = isFlatIn;
			}

			void SetNormal(Texture* n) {
				normalMap = n;
			}

			Texture* GetNormalMap() const {
				return normalMap;
			}

		protected:
			GameObject* parent;
			std::shared_ptr<Mesh> mesh;
			std::shared_ptr<Texture> texture;
			std::shared_ptr<Shader> shader;
			Vector4		colour = Vector4(1, 1, 0, 0.99);
			bool isFlat = false;
			//additional normal map option:
			Texture* normalMap;
		};
	}
}
