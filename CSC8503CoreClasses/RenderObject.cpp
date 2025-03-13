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
}

NCL::CSC8503::RenderObject::RenderObject(GameObject* parent, std::shared_ptr<Mesh> mesh, std::shared_ptr<MeshMaterial> mat)
{
    this->parent = parent;
    this->mesh = mesh;
    this->colour = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
    this->texture = mat->GetMaterialForLayer(0)->GetEntry("Diffuse");
    this->normalMap = mat->GetMaterialForLayer(0)->GetEntry("Bump");
    this->metallicMap = mat->GetMaterialForLayer(0)->GetEntry("Metallic");
}

RenderObject::~RenderObject() {

}
