#include "RenderObject.h"
#include "Mesh.h"

using namespace NCL::CSC8503;
using namespace NCL;

RenderObject::RenderObject(GameObject* parent, std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture> tex, std::shared_ptr<Shader> shader, std::shared_ptr<Texture> normal) {
	this->parent	= parent;
	this->mesh		= mesh;
	this->texture	= tex;
	this->shader	= shader;
	this->colour	= Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	this->normalMap = normal;
}

RenderObject::~RenderObject() {

}
