#ifndef _POSTPROCESSNODE
#define _POSTPROCESSNODE

#include "behavior_tree_builder.h"
#include "rendering_block.h"
#include "setting_ui.h"

namespace engine {
class PostProcessingNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[common::EnumDataBlockType::eRenderBlock]);
        assert(manager != nullptr);

        if (!is_initialized) {

            post_process.Initialize(GraphicsManager::Instance().device,
                                    GraphicsManager::Instance().device_context);

            GraphicsUtil::CreateConstBuffer(GraphicsManager::Instance().device,
                                            post_effects_consts_CPU,
                                            post_effects_consts_GPU);

            is_initialized = true;

        } else {

            GraphicsManager::Instance().SetPipelineState(
                Graphics::postProcessingPSO);
            post_process.Render(GraphicsManager::Instance().device,
                                GraphicsManager::Instance().device_context,
                                &manager->global_consts_CPU,
                                manager->global_consts_GPU);
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }

    bool is_initialized = false;
    PostEffectsConstants post_effects_consts_CPU;
    ComPtr<ID3D11Buffer> post_effects_consts_GPU;

    PostProcess post_process;
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