#ifndef _PIPELINEMANAGER
#define _PIPELINEMANAGER

#include "camera.h"
#include "dataBlock.h"
#include "graphics_manager.h"
#include "model.h"
#include "constant_buffer.h"
#include "post_process.h"

namespace engine {

class RenderingBlock : public common::IDataBlock, public common::INode {
  public:
    std::unique_ptr<Camera> camera;
    std::shared_ptr<Model> skybox;
    std::shared_ptr<Model> screen_square;
    std::shared_ptr<Model> light_spheres[MAX_LIGHTS];
    std::shared_ptr<Model> ground;
    std::map<int /*id*/, std::shared_ptr<Model>> models;

    //shared resource
    GlobalConstants m_globalConstsCPU;
    GlobalConstants m_reflectGlobalConstsCPU;
    GlobalConstants m_shadowGlobalConstsCPU[MAX_LIGHTS];
    ComPtr<ID3D11Buffer> m_globalConstsGPU;
    ComPtr<ID3D11Buffer> m_reflectGlobalConstsGPU;
    ComPtr<ID3D11Buffer> m_shadowGlobalConstsGPU[MAX_LIGHTS];

    //shared texture
    ComPtr<ID3D11ShaderResourceView> m_envSRV;
    ComPtr<ID3D11ShaderResourceView> m_irradianceSRV;
    ComPtr<ID3D11ShaderResourceView> m_specularSRV;
    ComPtr<ID3D11ShaderResourceView> m_brdfSRV;

    PostEffectsConstants m_postEffectsConstsCPU;
    ComPtr<ID3D11Buffer> m_postEffectsConstsGPU;
    PostProcess m_postProcess;

    //mirror
    std::shared_ptr<Model> m_mirror;
    DirectX::SimpleMath::Plane m_mirrorPlane;
    float m_mirrorAlpha = 1.0f; // opacity

    bool m_drawAsWire = false;
    bool m_lightRotate = false;
    
    void OnShow() override{

        auto device = GraphicsManager::Instance().device;
        auto context = GraphicsManager::Instance().device_context;

        ImGui::SetNextItemOpen(false, ImGuiCond_Once);
        if (ImGui::TreeNode("General")) {
            ImGui::Checkbox("Wireframe", &m_drawAsWire);
            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Skybox")) {
            ImGui::SliderFloat("Strength", &m_globalConstsCPU.strengthIBL, 0.0f,
                               5.0f);
            ImGui::RadioButton("Env", &m_globalConstsCPU.textureToDraw, 0);
            ImGui::SameLine();
            ImGui::RadioButton("Specular", &m_globalConstsCPU.textureToDraw, 1);
            ImGui::SameLine();
            ImGui::RadioButton("Irradiance", &m_globalConstsCPU.textureToDraw,
                               2);
            ImGui::SliderFloat("EnvLodBias", &m_globalConstsCPU.envLodBias,
                               0.0f, 10.0f);
            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Post Effects")) {
            int flag = 0;
            flag +=
                ImGui::RadioButton("Render", &m_postEffectsConstsCPU.mode, 1);
            ImGui::SameLine();
            flag +=
                ImGui::RadioButton("Depth", &m_postEffectsConstsCPU.mode, 2);
            flag += ImGui::SliderFloat(
                "DepthScale", &m_postEffectsConstsCPU.depthScale, 0.0, 1.0);
            flag += ImGui::SliderFloat(
                "Fog", &m_postEffectsConstsCPU.fogStrength, 0.0, 10.0);

            if (flag)
                GraphicsUtil::UpdateBuffer(device, context,
                                         m_postEffectsConstsCPU,
                                         m_postEffectsConstsGPU);

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Post Processing")) {
            int flag = 0;
            flag += ImGui::SliderFloat(
                "Bloom Strength",
                &m_postProcess.m_combineFilter.m_constData.strength, 0.0f,
                1.0f);
            flag += ImGui::SliderFloat(
                "Exposure", &m_postProcess.m_combineFilter.m_constData.option1,
                0.0f, 10.0f);
            flag += ImGui::SliderFloat(
                "Gamma", &m_postProcess.m_combineFilter.m_constData.option2,
                0.1f, 5.0f);

            if (flag) {
                m_postProcess.m_combineFilter.UpdateConstantBuffers(device,
                                                                    context);
            }
            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Mirror")) {

            ImGui::SliderFloat("Alpha", &m_mirrorAlpha, 0.0f, 1.0f);
            const float blendColor[4] = {m_mirrorAlpha, m_mirrorAlpha,
                                         m_mirrorAlpha, 1.0f};
            if (m_drawAsWire)
                Graphics::mirrorBlendWirePSO.SetBlendFactor(blendColor);
            else
                Graphics::mirrorBlendSolidPSO.SetBlendFactor(blendColor);

            //ImGui::SliderFloat("Metallic",
            //                   &m_mirror->m_materialConstsCPU.metallicFactor,
            //                   0.0f, 1.0f);
            //ImGui::SliderFloat("Roughness",
            //                   &m_mirror->m_materialConstsCPU.roughnessFactor,
            //                   0.0f, 1.0f);

            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Light")) {
            // ImGui::SliderFloat3("Position",
            // &m_globalConstsCPU.lights[0].position.x,
            //                     -5.0f, 5.0f);
            ImGui::SliderFloat("Halo Radius",
                               &m_globalConstsCPU.lights[1].haloRadius, 0.0f,
                               2.0f);
            ImGui::SliderFloat("Halo Strength",
                               &m_globalConstsCPU.lights[1].haloStrength, 0.0f,
                               1.0f);
            ImGui::SliderFloat("Radius", &m_globalConstsCPU.lights[1].radius,
                               0.0f, 0.5f);
            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Material")) {
            ImGui::SliderFloat("LodBias", &m_globalConstsCPU.lodBias, 0.0f,
                               10.0f);

            //int flag = 0;

            //flag += ImGui::SliderFloat(
            //    "Metallic", &m_mainObj->m_materialConstsCPU.metallicFactor,
            //    0.0f, 1.0f);
            //flag += ImGui::SliderFloat(
            //    "Roughness", &m_mainObj->m_materialConstsCPU.roughnessFactor,
            //    0.0f, 1.0f);
            //flag += ImGui::CheckboxFlags(
            //    "AlbedoTexture", &m_mainObj->m_materialConstsCPU.useAlbedoMap,
            //    1);
            //flag += ImGui::CheckboxFlags(
            //    "EmissiveTexture",
            //    &m_mainObj->m_materialConstsCPU.useEmissiveMap, 1);
            //flag += ImGui::CheckboxFlags(
            //    "Use NormalMapping",
            //    &m_mainObj->m_materialConstsCPU.useNormalMap, 1);
            //flag += ImGui::CheckboxFlags(
            //    "Use AO", &m_mainObj->m_materialConstsCPU.useAOMap, 1);
            //flag += ImGui::CheckboxFlags(
            //    "Use HeightMapping", &m_mainObj->m_meshConstsCPU.useHeightMap,
            //    1);
            //flag += ImGui::SliderFloat("HeightScale",
            //                           &m_mainObj->m_meshConstsCPU.heightScale,
            //                           0.0f, 0.1f);
            //flag += ImGui::CheckboxFlags(
            //    "Use MetallicMap",
            //    &m_mainObj->m_materialConstsCPU.useMetallicMap, 1);
            //flag += ImGui::CheckboxFlags(
            //    "Use RoughnessMap",
            //    &m_mainObj->m_materialConstsCPU.useRoughnessMap, 1);

            //if (flag) {
            //    m_mainObj->UpdateConstantBuffers(m_device, m_context);
            //}

            //ImGui::Checkbox("Draw Normals", &m_mainObj->m_drawNormals);

            ImGui::TreePop();
        }
    }
};
} // namespace dx11
#endif
