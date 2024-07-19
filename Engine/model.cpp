#pragma warning(disable : 6385)
#pragma warning(disable : 6386)

#include "model.h"

namespace engine {

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

    if (ImGui::RadioButton("Use Phong",
                           render_mode == EnumRenderMode::eLight)) {
        render_mode = EnumRenderMode::eLight;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Use PBR",
                           render_mode ==
                               EnumRenderMode::ePhysicallyBasedRendering)) {
        render_mode = EnumRenderMode::ePhysicallyBasedRendering;
    }
}
} // namespace engine