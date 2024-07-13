#pragma warning(disable : 6385)
#pragma warning(disable : 6386)

#include "model.h"

using namespace dx11;

Model::Model() {}

Model::~Model() {}

int Model::GetIndexCount() {
    int count = 0;
    for (auto mesh : meshes)
        count += mesh->indices.size();
    return count;
}

void Model::OnShow() {
    ImGui::Text("Transform");
    ImGui::SliderFloat3("Translation", &translation.x, -2.0f, 2.0f);
    ImGui::SliderFloat("Rotation", &rotation.y, -3.14f, 3.14f);
    ImGui::SliderFloat3("Scaling", &scaling.x, 0.1f, 4.0f);

    switch (shader_type) {
    case EnumShaderType::ePhong: {
        break;
    }
    case EnumShaderType::eImageBased: {
        break;
    }
    case EnumShaderType::ePhysicallyBased: {
        break;
    }
    default:
        break;
    }

    // phong_shader_source->Show();

}
