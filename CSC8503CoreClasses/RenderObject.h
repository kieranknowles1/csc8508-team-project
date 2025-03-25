#pragma once
#include "Texture.h"
#include "Mesh.h"
#include "Buffer.h"
#include "MeshAnimation.h"
#include "MeshMaterial.h"

namespace NCL {
	using namespace NCL::Rendering;

	class Material;

	namespace CSC8503 {
		class GameObject;
		using namespace Maths;

		class RenderObject
		{
		public:
			RenderObject(GameObject* parent, std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture> tex, std::shared_ptr<Texture> normal = nullptr);
            RenderObject(GameObject* parent, std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> mat);
			~RenderObject();

			Mesh* GetMesh() const {
				return mesh.get();
			}

			void SetColour(const Vector4& c) {
				colour = c;
			}

			std::shared_ptr<Material> getMaterial() const { return material; }
			void setMaterial(std::shared_ptr<Material> mat) { material = mat; }

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

			bool GetTexRepeating() const {
				return texRepeating;
			}

			void SetTexRepeating(bool tr) {
				texRepeating = tr;
			}

			float GetTexScaleMultiplier() const {
				return texScaleMultiplier;
			}

			void SetTexScaleMultiplier(float f) { //in case an individual object's texture scale needs to be modified
				texScaleMultiplier = f;
			}

			void SetGPUBuffer(Buffer* buf) {
				buffer = buf;
			}
			Buffer* GetGPUBuffer() {
				return buffer;
			}

			MeshAnimation* GetAnimation() const {
				return test;
			}

			void SetAnimation(MeshAnimation* name) {
				test = name; 
			}

		protected:
			GameObject* parent;
			Buffer* buffer = nullptr;
			std::shared_ptr<Mesh> mesh;
			Vector4		colour = Vector4(1, 1, 0, 0.99);
			bool isFlat = false;
			bool texRepeating = false; // added to allow repeating textures per object
			float texScaleMultiplier = 0.005f; //unless set to something else, all scaled textures will be scaled with this and their renderScale

			//Animation stuff:
			MeshAnimation* test = nullptr;
		


			std::shared_ptr<Material> material;
		};
	}
}
