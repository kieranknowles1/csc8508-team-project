#include "RenderObject.h"
#include "Mesh.h"

using namespace NCL::CSC8503;
using namespace NCL;

RenderObject::RenderObject(GameObject* parent, std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture> tex, std::shared_ptr<Texture> normal) {
	this->parent	= parent;
	this->mesh		= mesh;
	this->texture	= tex;
	this->colour	= Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	this->normalMap = normal;

	//set materials instead of textures:
	/*this->material = new MeshMaterial("/MaleGuard/Male_Guard.mat"); //should the .mat extension be included?
	for (int i = 0; i < this->mesh->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = this->material->GetMaterialForLayer(i);   
	}*/
}

RenderObject::~RenderObject() { 

}
