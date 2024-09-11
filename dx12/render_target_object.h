#ifndef _RENDER_TARGET_OBJECT
#define _RENDER_TARGET_OBJECT

#include "camera.h"
#include "foundation/info.h"

namespace graphics {
class RenderTargetObject : public foundation::IInfo {
  public:
    RenderTargetObject() {}
    std::unique_ptr<Camera> camera;
    // std::shared_ptr<Ground> ground;
    // std::shared_ptr<Model> player;
    std::map<int /*id*/, std::shared_ptr<Model>> objects;

    // void OnShow() override {
    //     ImGui::SetNextItemOpen(false, ImGuiCond_Once);
    //     if (ImGui::TreeNode("General")) {
    //         ImGui::Checkbox("Wireframe", &draw_wire);
    //         ImGui::TreePop();
    //     }

    //    ImGui::SetNextItemOpen(false, ImGuiCond_Once);
    //    if (ImGui::TreeNode("Skybox")) {
    //        ImGui::SliderFloat("Strength", &global_consts_CPU.strengthIBL,
    //        0.0f,
    //                           5.0f);
    //        ImGui::RadioButton("Env", &global_consts_CPU.textureToDraw, 0);
    //        ImGui::SameLine();
    //        ImGui::RadioButton("Specular", &global_consts_CPU.textureToDraw,
    //        1); ImGui::SameLine(); ImGui::RadioButton("Irradiance",
    //        &global_consts_CPU.textureToDraw,
    //                           2);
    //        ImGui::SliderFloat("EnvLodBias", &global_consts_CPU.envLodBias,
    //                           0.0f, 10.0f);
    //        ImGui::TreePop();
    //    }

    //    ImGui::SetNextItemOpen(false, ImGuiCond_Once);
    //    if (ImGui::TreeNode("Mirror")) {

    //        ImGui::SliderFloat("Alpha", &ground->mirror_alpha, 0.0f, 1.0f);
    //        const float blendColor[4] = {ground->mirror_alpha,
    //                                     ground->mirror_alpha,
    //                                     ground->mirror_alpha, 1.0f};
    //        if (draw_wire)
    //            dx11::pso::mirrorBlendWirePSO.SetBlendFactor(blendColor);
    //        else
    //            dx11::pso::mirrorBlendSolidPSO.SetBlendFactor(blendColor);

    //        auto renderer = (MeshRenderer *)ground->mirror->GetComponent(
    //            EnumComponentType::eRenderer);
    //        ImGui::SliderFloat(
    //            "Metallic",
    //            &renderer->material_consts.GetCpu().metallicFactor,
    //            0.0f, 1.0f);
    //        ImGui::SliderFloat(
    //            "Roughness",
    //            &renderer->material_consts.GetCpu().roughnessFactor, 0.0f,
    //            1.0f);

    //        ImGui::TreePop();
    //    }

    //    ImGui::SetNextItemOpen(false, ImGuiCond_Once);
    //    if (ImGui::TreeNode("Light")) {
    //        ImGui::Checkbox("Light Rotate", &light_rotate);
    //        ImGui::DragFloat3("Position",
    //                          &global_consts_CPU.lights[0].position.x, 0.1f,
    //                          -5.0f, 5.0f);
    //        ImGui::SliderFloat("Halo Radius",
    //                           &global_consts_CPU.lights[1].haloRadius, 0.0f,
    //                           2.0f);
    //        ImGui::SliderFloat("Halo Strength",
    //                           &global_consts_CPU.lights[1].haloStrength,
    //                           0.0f, 1.0f);
    //        ImGui::SliderFloat("Radius", &global_consts_CPU.lights[1].radius,
    //                           0.0f, 0.5f);
    //        ImGui::TreePop();
    //    }

    //    ImGui::SetNextItemOpen(false, ImGuiCond_Once);
    //    if (ImGui::TreeNode("Material")) {
    //        ImGui::SliderFloat("LodBias", &global_consts_CPU.lodBias, 0.0f,
    //                           10.0f);
    //        ImGui::TreePop();
    //    }
    //}
};
} // namespace core
#endif
