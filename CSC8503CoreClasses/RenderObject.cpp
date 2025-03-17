#include "RenderObject.h"
#include "Mesh.h"

using namespace NCL::CSC8503;
using namespace NCL;

RenderObject::RenderObject(GameObject* parent, std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture> tex, std::shared_ptr<Texture> normal) {
	this->parent	= parent;
	this->mesh		= mesh;
	this->colour	= Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    this->textures.push_back(tex);
    this->normalMaps.push_back(normal);
}

NCL::CSC8503::RenderObject::RenderObject(GameObject* parent, std::shared_ptr<Mesh> mesh, std::shared_ptr<MeshMaterial> mat)
{
    this->parent = parent;
    this->mesh = mesh;
    this->colour = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

    // Check if the number of submeshes matches the material layers
    int subMeshCount = mesh->GetSubMeshCount();
    int materialCount = mat->GetLayerCount();

    // Assign materials per submesh
    for (int i = 0; i < subMeshCount; i++) {
        if (i < materialCount) { // Ensure we don't go out of bounds
            auto materialLayer = mat->GetMaterialForLayer(i);
            if (materialLayer) {
                textures.push_back(materialLayer->GetEntry("Diffuse"));
                normalMaps.push_back(materialLayer->GetEntry("Bump"));
                metallicMaps.push_back(materialLayer->GetEntry("Metallic"));
            }
            else {
                textures.push_back(nullptr);
                normalMaps.push_back(nullptr);
                metallicMaps.push_back(nullptr);
            }
        }
    }
}

RenderObject::~RenderObject() {

}
