#include "Material.h"

#include "ResourceManager.h"
#include "MeshMaterial.h"

namespace NCL {

Material::Material(CSC8503::ResourceManager* rm, const std::string& filename) {
    MeshMaterial file(filename);
    for (int i = 0; i < file.GetLayerCount(); i++) {
        Layer l;
        auto layer = file.GetMaterialForLayer(i);

        const std::string* out;
        if (layer->GetEntry("Diffuse", &out)) {
            l.diffuse = rm->getTextures().get(*out);
        }
        if (layer->GetEntry("Bump", &out)) {
            l.normal = rm->getTextures().get(*out);
        }
        if (layer->GetEntry("InvertY", &out)) {
            l.invertY = *out == "true";
        }
        if (layer->GetEntry("UseColor", &out)) {
            l.useColor = *out == "true";
        }
        layers.push_back(l);
    }
}

}