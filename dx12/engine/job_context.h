#ifndef _PIPELINEMANAGER
#define _PIPELINEMANAGER

#include "../foundation/setting_ui.h"
#include "../foundation/info.h"
#include "../graphics/command_pool.h"
#include "../graphics/graphics_util.h"
#include "input.h"
#include "camera.h"
#include "constant_buffer.h"
// #include "dataBlock.h"
// #include "ground.h"
// #include "mesh_renderer.h"
// #include "model.h"
// #include "skybox.h"

namespace core {
enum EnumStageType {
    eInitialize = 0,
    eUpdate = 1,
    eRender = 2,
};

class RenderCondition : public foundation::IInfo {
  public:
    RenderCondition()
        : global_consts_GPU(0), sampler_heap(0), dt(0.0f), draw_wire(false),
          light_rotate(false), stage_type(EnumStageType::eInitialize) {
        input = std::make_unique<foundation::Input>();
        gui = std::make_shared<foundation::SettingUi>();
        command_pool = std::make_shared<dx12::CommandPool>();
    }
    std::shared_ptr<foundation::Input> input;
    std::shared_ptr<foundation::SettingUi> gui;
    std::shared_ptr<dx12::CommandPool> command_pool;

    GlobalConstants global_consts_CPU;
    ComPtr<ID3D12Resource> global_consts_GPU;
    ComPtr<ID3D12DescriptorHeap> sampler_heap;

    float dt;
    bool draw_wire;
    bool light_rotate;
    EnumStageType stage_type;
};
class RenderTarget : public foundation::IInfo {
  public:
    RenderTarget() {}
    std::unique_ptr<Camera> camera;
    // std::shared_ptr<Ground> ground;
    // std::shared_ptr<Skybox> skybox;
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
