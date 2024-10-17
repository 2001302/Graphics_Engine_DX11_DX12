#ifndef _RENDER_CONDITION
#define _RENDER_CONDITION

#include "constant_buffer.h"
#include "dynamic_descriptor_heap.h"
#include "gpu_resource.h"
#include "mesh_util.h"
#include "sampler_state.h"
#include "texture_2d.h"
#include "texture_cube.h"
#include <info.h>
#include <setting_ui.h>

namespace graphics {
enum EnumStageType {
    eInitialize = 0,
    eUpdate = 1,
    eRender = 2,
};

class RenderCondition : public common::IInfo {
  public:
    RenderCondition()
        : delta_time(0.0f), draw_wire(false), light_rotate(false),
          stage_type(EnumStageType::eInitialize), shared_sampler(0),
          shadow_texture(0) {}
    ~RenderCondition() {

        if (shared_sampler) {
            delete shared_sampler;
            shared_sampler = 0;
        }
    }

    GpuResourceList *shadow_texture; // t14~t16
    SamplerState *shared_sampler;
    ConstantBuffer<GlobalConstants> global_consts;

    float delta_time;
    bool draw_wire;
    bool light_rotate;
    EnumStageType stage_type;

    void OnShow() override {
        ImGui::SetNextItemOpen(false, ImGuiCond_Once);
        if (ImGui::TreeNode("General")) {
            ImGui::Checkbox("Wireframe", &draw_wire);
            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(false, ImGuiCond_Once);
        if (ImGui::TreeNode("Skybox")) {
            ImGui::SliderFloat("Strength", &global_consts.GetCpu().strengthIBL,
                               0.0f, 5.0f);
            ImGui::RadioButton("Env", &global_consts.GetCpu().textureToDraw, 0);
            ImGui::SameLine();
            ImGui::RadioButton("Specular",
                               &global_consts.GetCpu().textureToDraw, 1);
            ImGui::SameLine();
            ImGui::RadioButton("Irradiance",
                               &global_consts.GetCpu().textureToDraw, 2);
            ImGui::SliderFloat("EnvLodBias", &global_consts.GetCpu().envLodBias,
                               0.0f, 10.0f);
            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(false, ImGuiCond_Once);
        if (ImGui::TreeNode("Mirror")) {

            // ImGui::SliderFloat("Alpha", &ground->mirror_alpha, 0.0f, 1.0f);
            // const float blendColor[4] = {ground->mirror_alpha,
            //                              ground->mirror_alpha,
            //                              ground->mirror_alpha, 1.0f};
            // if (draw_wire)
            //     dx11::pso::mirrorBlendWirePSO.SetBlendFactor(blendColor);
            // else
            //     dx11::pso::mirrorBlendSolidPSO.SetBlendFactor(blendColor);

            // auto renderer = (MeshRenderer *)ground->mirror->GetComponent(
            //     EnumComponentType::eRenderer);
            // ImGui::SliderFloat(
            //     "Metallic",
            //     &renderer->material_consts.GetCpu().metallicFactor,
            //     0.0f, 1.0f);
            // ImGui::SliderFloat(
            //     "Roughness",
            //     &renderer->material_consts.GetCpu().roughnessFactor, 0.0f,
            //     1.0f);

            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(false, ImGuiCond_Once);
        if (ImGui::TreeNode("Light")) {
            ImGui::Checkbox("Light Rotate", &light_rotate);
            ImGui::DragFloat3("Position",
                              &global_consts.GetCpu().lights[0].position.x,
                              0.1f, -5.0f, 5.0f);
            ImGui::SliderFloat("Halo Radius",
                               &global_consts.GetCpu().lights[1].haloRadius,
                               0.0f, 2.0f);
            ImGui::SliderFloat("Halo Strength",
                               &global_consts.GetCpu().lights[1].haloStrength,
                               0.0f, 1.0f);
            ImGui::SliderFloat(
                "Radius", &global_consts.GetCpu().lights[1].radius, 0.0f, 0.5f);
            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(false, ImGuiCond_Once);
        if (ImGui::TreeNode("Material")) {
            ImGui::SliderFloat("LodBias", &global_consts.GetCpu().lodBias, 0.0f,
                               10.0f);
            ImGui::TreePop();
        }
    }
};
} // namespace graphics
#endif
