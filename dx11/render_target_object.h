#ifndef _PIPELINEMANAGER
#define _PIPELINEMANAGER

#include "camera.h"
#include "constant_buffer.h"
#include "graphics_util.h"
#include "mesh_renderer.h"
#include "mirror_renderer.h"
#include <structure/dataBlock.h>
#include <structure/info.h>
#include <structure/model.h>

namespace graphics {
enum EnumStageType {
    eInitialize = 0,
    eUpdate = 1,
    eRender = 2,
};

class RenderTargetObject : public common::IInfo {
  public:
    RenderTargetObject() : delta_time(0.0f), stage_type() {
        skybox = std::make_shared<common::Model>();
        ground = std::make_shared<common::Model>();
    }

    std::unique_ptr<Camera> camera;
    std::shared_ptr<common::Model> ground;
    std::shared_ptr<common::Model> skybox;
    std::shared_ptr<common::Model> player;
    std::map<int /*id*/, std::shared_ptr<common::Model>> objects;

    // condition
    GlobalConstants global_consts_CPU;
    ComPtr<ID3D11Buffer> global_consts_GPU;
    float delta_time;
    bool draw_wire = false;
    bool light_rotate = false;
    EnumStageType stage_type;

    void OnShow() override {
        ImGui::SetNextItemOpen(false, ImGuiCond_Once);
        if (ImGui::TreeNode("General")) {
            ImGui::Checkbox("Wireframe", &draw_wire);
            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(false, ImGuiCond_Once);
        if (ImGui::TreeNode("Skybox")) {
            ImGui::SliderFloat("Strength", &global_consts_CPU.strengthIBL, 0.0f,
                               5.0f);
            ImGui::RadioButton("Env", &global_consts_CPU.textureToDraw, 0);
            ImGui::SameLine();
            ImGui::RadioButton("Specular", &global_consts_CPU.textureToDraw, 1);
            ImGui::SameLine();
            ImGui::RadioButton("Irradiance", &global_consts_CPU.textureToDraw,
                               2);
            ImGui::SliderFloat("EnvLodBias", &global_consts_CPU.envLodBias,
                               0.0f, 10.0f);
            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(false, ImGuiCond_Once);
        if (ImGui::TreeNode("Mirror")) {

            MirrorRenderer *mirror = nullptr;
            if (ground->TryGet(mirror)) {
                float mirror_alpha = mirror->GetMirrorAlpha();
                ImGui::SliderFloat("Alpha", &mirror_alpha, 0.0f, 1.0f);
                const float blendColor[4] = {mirror_alpha, mirror_alpha,
                                             mirror_alpha, 1.0f};
                mirror->SetMirrorAlpha(mirror_alpha);
                if (draw_wire)
                    graphics::pipeline::mirrorBlendWirePSO.SetBlendFactor(
                        blendColor);
                else
                    graphics::pipeline::mirrorBlendSolidPSO.SetBlendFactor(
                        blendColor);

                ImGui::SliderFloat(
                    "Metallic",
                    &mirror->material_consts.GetCpu().metallicFactor, 0.0f,
                    1.0f);
                ImGui::SliderFloat(
                    "Roughness",
                    &mirror->material_consts.GetCpu().roughnessFactor, 0.0f,
                    1.0f);
            }

            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(false, ImGuiCond_Once);
        if (ImGui::TreeNode("Light")) {
            ImGui::Checkbox("Light Rotate", &light_rotate);
            ImGui::DragFloat3("Position",
                              &global_consts_CPU.lights[0].position.x, 0.1f,
                              -5.0f, 5.0f);
            ImGui::SliderFloat("Halo Radius",
                               &global_consts_CPU.lights[1].haloRadius, 0.0f,
                               2.0f);
            ImGui::SliderFloat("Halo Strength",
                               &global_consts_CPU.lights[1].haloStrength, 0.0f,
                               1.0f);
            ImGui::SliderFloat("Radius", &global_consts_CPU.lights[1].radius,
                               0.0f, 0.5f);
            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(false, ImGuiCond_Once);
        if (ImGui::TreeNode("Material")) {
            ImGui::SliderFloat("LodBias", &global_consts_CPU.lodBias, 0.0f,
                               10.0f);
            ImGui::TreePop();
        }
    }
};
} // namespace graphics
#endif
