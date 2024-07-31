#ifndef _POSTPROCESSNODE
#define _POSTPROCESSNODE

#include "behavior_tree_builder.h"
#include "image_filter.h"
#include "rendering_block.h"
#include "setting_ui.h"
#include "tone_mapping.h"

namespace engine {
/// <summary>
/// Bloom + ToneMapping
/// </summary>
class PostProcessingNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[common::EnumDataBlockType::eRenderBlock]);
        assert(manager != nullptr);

        auto device = GraphicsManager::Instance().device;
        auto context = GraphicsManager::Instance().device_context;

        switch (manager->stage_type) {
        case EnumStageType::eInitialize: {
            const_data.strength = 0.5f; // Bloom strength

            GraphicsUtil::CreateConstBuffer(device, const_data, const_buffer);
            GraphicsUtil::UpdateBuffer(device, context, const_data,
                                       const_buffer);
            // bloom
            GraphicsUtil::CreateUATexture(
                device, common::Env::Instance().screen_width,
                common::Env::Instance().screen_height,
                DXGI_FORMAT_R16G16B16A16_FLOAT, bright_pass_buffer,
                bright_pass_RTV, bright_pass_SRV, bright_pass_UAV);

            GraphicsUtil::CreateUATexture(
                device, common::Env::Instance().screen_width,
                common::Env::Instance().screen_height,
                DXGI_FORMAT_R16G16B16A16_FLOAT, blur_vertical_buffer,
                blur_vertical_RTV, blur_vertical_SRV, blur_vertical_UAV);

            GraphicsUtil::CreateUATexture(
                device, common::Env::Instance().screen_width,
                common::Env::Instance().screen_height,
                DXGI_FORMAT_R16G16B16A16_FLOAT, blur_horizontal_buffer,
                blur_horizontal_RTV, blur_horizontal_SRV, blur_horizontal_UAV);

            // tone mapping
            tone_mapping.Initialize(device, context);
            break;
        }
        case EnumStageType::eRender: {
            // bloom filter
            // bright pass->blur vertical->blur horizontal->composite
            bright_pass.Render(context, Graphics::brightPassCS, const_buffer,
                               {GraphicsManager::Instance().resolved_SRV},
                               bright_pass_UAV);

            blur_vertical.Render(context, Graphics::blurVerticalCS,
                                 const_buffer, {bright_pass_SRV},
                                 blur_vertical_UAV);

            blur_horizontal.Render(context, Graphics::blurHorizontalCS,
                                   const_buffer, {blur_vertical_SRV},
                                   blur_horizontal_UAV);

            bloom_composite.Render(
                context, Graphics::bloomComposite, const_buffer,
                {GraphicsManager::Instance().resolved_SRV, blur_horizontal_SRV},
                bright_pass_UAV);

            context->CopyResource(
                GraphicsManager::Instance().resolved_buffer.Get(),
                bright_pass_buffer.Get());

            // tone mapping
            GraphicsManager::Instance().SetPipelineState(
                Graphics::postProcessingPSO);
            tone_mapping.Render(device, context);
            break;
        }
        default:
            break;
        }
        return common::EnumBehaviorTreeStatus::eSuccess;
    }

    ImageFilterConstData const_data = {};
    ComPtr<ID3D11Buffer> const_buffer;

    ImageFilter bright_pass;
    ImageFilter blur_vertical;
    ImageFilter blur_horizontal;
    ImageFilter bloom_composite;

    ComPtr<ID3D11Texture2D> bright_pass_buffer;
    ComPtr<ID3D11RenderTargetView> bright_pass_RTV;
    ComPtr<ID3D11ShaderResourceView> bright_pass_SRV;
    ComPtr<ID3D11UnorderedAccessView> bright_pass_UAV;

    ComPtr<ID3D11Texture2D> blur_vertical_buffer;
    ComPtr<ID3D11RenderTargetView> blur_vertical_RTV;
    ComPtr<ID3D11ShaderResourceView> blur_vertical_SRV;
    ComPtr<ID3D11UnorderedAccessView> blur_vertical_UAV;

    ComPtr<ID3D11Texture2D> blur_horizontal_buffer;
    ComPtr<ID3D11RenderTargetView> blur_horizontal_RTV;
    ComPtr<ID3D11ShaderResourceView> blur_horizontal_SRV;
    ComPtr<ID3D11UnorderedAccessView> blur_horizontal_UAV;

    ToneMapping tone_mapping;
};

// ImGui::SetNextItemOpen(true, ImGuiCond_Once);
// if (ImGui::TreeNode("Post Effects")) {
//     int flag = 0;
//     flag += ImGui::RadioButton("Render", &post_effects_consts_CPU.mode, 1);
//     ImGui::SameLine();
//     flag += ImGui::RadioButton("Depth", &post_effects_consts_CPU.mode, 2);
//     flag += ImGui::SliderFloat("DepthScale",
//                                &post_effects_consts_CPU.depthScale,
//                                0.0, 1.0);
//     flag += ImGui::SliderFloat("Fog", &post_effects_consts_CPU.fogStrength,
//     0.0,
//                                10.0);
//
//     if (flag)
//         GraphicsUtil::UpdateBuffer(device, context, post_effects_consts_CPU,
//                                    post_effects_consts_GPU);
//
//     ImGui::TreePop();
// }
//
// if (ImGui::TreeNode("Post Processing")) {
//      int flag = 0;
//      flag += ImGui::SliderFloat(
//          "Bloom Strength",
//          &post_process.m_combineFilter.m_constData.strength, 0.0f,
//          1.0f);
//      flag += ImGui::SliderFloat(
//          "Exposure", &post_process.m_combineFilter.m_constData.option1,
//          0.0f, 10.0f);
//      flag += ImGui::SliderFloat(
//          "Gamma", &post_process.m_combineFilter.m_constData.option2,
//          0.1f, 5.0f);
//
//      if (flag) {
//          post_process.m_combineFilter.UpdateConstantBuffers(device,
//                                                              context);
//      }
//     ImGui::TreePop();
// }

// class InitializePostProcessingNode : public common::BehaviorActionNode {
//     common::EnumBehaviorTreeStatus OnInvoke() override {
//
//         auto manager = dynamic_cast<RenderingBlock *>(
//             data_block[common::EnumDataBlockType::eRenderBlock]);
//         assert(manager != nullptr);
//
//         manager->post_process.Initialize(
//             GraphicsManager::Instance().device,
//             GraphicsManager::Instance().device_context);
//
//         return common::EnumBehaviorTreeStatus::eSuccess;
//     }
// };
//
// class DrawPostProcessingNode : public common::BehaviorActionNode {
//     common::EnumBehaviorTreeStatus OnInvoke() override {
//
//         auto manager = dynamic_cast<RenderingBlock *>(
//             data_block[common::EnumDataBlockType::eRenderBlock]);
//         assert(manager != nullptr);
//
//         //// PostEffects
//         //
//         GraphicsManager::Instance().SetPipelineState(Graphics::postEffectsPSO);
//
//         // std::vector<ID3D11ShaderResourceView *> postEffectsSRVs = {
//         //     GraphicsManager::Instance().resolved_SRV.Get(), nullptr};
//
//         //
//         GraphicsManager::Instance().SetGlobalConsts(manager->global_consts_GPU);
//
//         //// post effect textures: start from register(20)
//         // GraphicsManager::Instance().device_context->PSSetShaderResources(
//         //     20, UINT(postEffectsSRVs.size()), postEffectsSRVs.data());
//         // //GraphicsManager::Instance().device_context->OMSetRenderTargets(
//         // //    1,
//         GraphicsManager::Instance().postEffectsRTV.GetAddressOf(),
//         // //    NULL);
//         // GraphicsManager::Instance().device_context->OMSetRenderTargets(
//         //     1, GraphicsManager::Instance().resolved_RTV.GetAddressOf(),
//         //     NULL);
//
//         // GraphicsManager::Instance().device_context->PSSetConstantBuffers(
//         //     3, 1, manager->post_effects_consts_GPU.GetAddressOf());
//
//         // if (true) {
//         //     Renderer *renderer = nullptr;
//         // manager->screen_square->GetComponent(EnumComponentType::eRenderer,
//         //                                           (Component
//         **)(&renderer));
//         //     renderer->Render(GraphicsManager::Instance().device_context);
//         // }
//
//         GraphicsManager::Instance().SetPipelineState(
//             Graphics::postProcessingPSO);
//         manager->post_process.Render(GraphicsManager::Instance().device,
//                                      GraphicsManager::Instance().device_context,
//                                      &manager->global_consts_CPU,
//                                      manager->global_consts_GPU);
//
//         return common::EnumBehaviorTreeStatus::eSuccess;
//     }
// };

} // namespace engine

#endif