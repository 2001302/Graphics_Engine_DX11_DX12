#ifndef _PIPELINEMANAGER
#define _PIPELINEMANAGER

#include "camera.h"
#include "constant_buffer.h"
#include "dataBlock.h"
#include "graphics_manager.h"
#include "model.h"

namespace engine {
enum EnumStageType {
    eInitialize = 0,
    eUpdate = 1,
    eRender = 2,
};

class RenderingBlock : public common::IDataBlock,
    public common::INode {
  public:
    float dt;
    bool draw_wire = false;
    bool light_rotate = false;
    EnumStageType stage_type;

    std::unique_ptr<Camera> camera;
    std::shared_ptr<Model> skybox;
    std::shared_ptr<Model> screen_square;
    std::shared_ptr<Model> light_spheres[MAX_LIGHTS];
    std::shared_ptr<Model> ground;
    std::map<int /*id*/, std::shared_ptr<Model>> models;

    // shared resource
    GlobalConstants global_consts_CPU;
    GlobalConstants reflect_global_consts_CPU;
    GlobalConstants shadow_global_consts_CPU[MAX_LIGHTS];
    ComPtr<ID3D11Buffer> global_consts_GPU;
    ComPtr<ID3D11Buffer> reflect_global_consts_GPU;
    ComPtr<ID3D11Buffer> shadow_global_consts_GPU[MAX_LIGHTS];

    // shared texture
    ComPtr<ID3D11ShaderResourceView> env_SRV;
    ComPtr<ID3D11ShaderResourceView> irradiance_SRV;
    ComPtr<ID3D11ShaderResourceView> specular_SRV;
    ComPtr<ID3D11ShaderResourceView> brdf_SRV;

    // mirror
    std::shared_ptr<Model> mirror;
    DirectX::SimpleMath::Plane mirror_plane;
    float mirror_alpha = 1.0f; // opacity

    void OnShow() override {

        auto device = GraphicsManager::Instance().device;
        auto context = GraphicsManager::Instance().device_context;

        ImGui::SetNextItemOpen(false, ImGuiCond_Once);
        if (ImGui::TreeNode("General")) {
            ImGui::Checkbox("Wireframe", &draw_wire);
            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
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

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Mirror")) {

            ImGui::SliderFloat("Alpha", &mirror_alpha, 0.0f, 1.0f);
            const float blendColor[4] = {mirror_alpha, mirror_alpha,
                                         mirror_alpha, 1.0f};
            if (draw_wire)
                Graphics::mirrorBlendWirePSO.SetBlendFactor(blendColor);
            else
                Graphics::mirrorBlendSolidPSO.SetBlendFactor(blendColor);

            // ImGui::SliderFloat("Metallic",
            //                    &m_mirror->m_materialConstsCPU.metallicFactor,
            //                    0.0f, 1.0f);
            // ImGui::SliderFloat("Roughness",
            //                    &m_mirror->m_materialConstsCPU.roughnessFactor,
            //                    0.0f, 1.0f);

            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Light")) {
            // ImGui::SliderFloat3("Position",
            // &m_globalConstsCPU.lights[0].position.x,
            //                     -5.0f, 5.0f);
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

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Material")) {
            ImGui::SliderFloat("LodBias", &global_consts_CPU.lodBias, 0.0f,
                               10.0f);

            // int flag = 0;

            // flag += ImGui::SliderFloat(
            //     "Metallic", &m_mainObj->m_materialConstsCPU.metallicFactor,
            //     0.0f, 1.0f);
            // flag += ImGui::SliderFloat(
            //     "Roughness", &m_mainObj->m_materialConstsCPU.roughnessFactor,
            //     0.0f, 1.0f);
            // flag += ImGui::CheckboxFlags(
            //     "AlbedoTexture",
            //     &m_mainObj->m_materialConstsCPU.useAlbedoMap, 1);
            // flag += ImGui::CheckboxFlags(
            //     "EmissiveTexture",
            //     &m_mainObj->m_materialConstsCPU.useEmissiveMap, 1);
            // flag += ImGui::CheckboxFlags(
            //     "Use NormalMapping",
            //     &m_mainObj->m_materialConstsCPU.useNormalMap, 1);
            // flag += ImGui::CheckboxFlags(
            //     "Use AO", &m_mainObj->m_materialConstsCPU.useAOMap, 1);
            // flag += ImGui::CheckboxFlags(
            //     "Use HeightMapping",
            //     &m_mainObj->m_meshConstsCPU.useHeightMap, 1);
            // flag += ImGui::SliderFloat("HeightScale",
            //                            &m_mainObj->m_meshConstsCPU.heightScale,
            //                            0.0f, 0.1f);
            // flag += ImGui::CheckboxFlags(
            //     "Use MetallicMap",
            //     &m_mainObj->m_materialConstsCPU.useMetallicMap, 1);
            // flag += ImGui::CheckboxFlags(
            //     "Use RoughnessMap",
            //     &m_mainObj->m_materialConstsCPU.useRoughnessMap, 1);

            // if (flag) {
            //     m_mainObj->UpdateConstantBuffers(m_device, m_context);
            // }

            // ImGui::Checkbox("Draw Normals", &m_mainObj->m_drawNormals);

            ImGui::TreePop();
        }
    }
};
} // namespace engine
#endif
