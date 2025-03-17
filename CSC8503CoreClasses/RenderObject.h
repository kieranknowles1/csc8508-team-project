#pragma once
#include "Texture.h"
#include "Mesh.h"
#include "Buffer.h"
#include "MeshAnimation.h"

namespace NCL {
	using namespace NCL::Rendering;

	namespace CSC8503 {
		class GameObject;
		using namespace Maths;

		class RenderObject
		{
		public:
			RenderObject(GameObject* parent, std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture> tex, std::shared_ptr<Texture> normal = nullptr);
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

			bool GetHasNormal() const {
				return normalMap != nullptr;
			}

			void SetIsFlat(bool isFlatIn) {
				isFlat = isFlatIn;
			}

			void SetNormal(std::shared_ptr<Texture> n) {
				normalMap = n;
			}

			Texture* GetNormalMap() const {
				return normalMap.get();
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
				test = name; //idk if this is how it should be handled. Might also be an issue if changing where the pointer is pointing
			}

			void SetMesh(std::shared_ptr<Mesh> M) { //Temporarily added
				mesh = M;
			}

		protected:
			GameObject* parent;
			Buffer* buffer;
			std::shared_ptr<Mesh> mesh;
			std::shared_ptr<Texture> texture;
			Vector4 colour = Vector4(1, 1, 1, 1);
			bool isFlat = false;
			bool texRepeating = false; // added to allow repeating textures per object
			float texScaleMultiplier = 0.005f; //unless set to something else, all scaled textures will be scaled with this and their renderScale
			//additional normal map option:
			std::shared_ptr<Texture> normalMap;

			//Animation stuff:
			MeshAnimation* test; //InitPlayer will set animated from gameObject to true
			//also might be good to have something like if animated, use material and mattextures and set all the textures for each submesh
			//doesn't neccessarily matter if a mesh has a texture if its never send to shader. Can just access matTextures instead
		};
	}
}
