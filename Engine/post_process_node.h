#ifndef _POSTPROCESSNODE
#define _POSTPROCESSNODE

#include "behavior_tree_builder.h"
#include "rendering_block.h"
#include "setting_ui.h"

namespace engine {
class InitializePostProcessingNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[common::EnumDataBlockType::eRenderBlock]);
        assert(manager != nullptr);

        manager->post_process.Initialize(
            GraphicsManager::Instance().device,
            GraphicsManager::Instance().device_context);

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class DrawPostProcessingNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[common::EnumDataBlockType::eRenderBlock]);
        assert(manager != nullptr);

        //// PostEffects
        // GraphicsManager::Instance().SetPipelineState(Graphics::postEffectsPSO);

        // std::vector<ID3D11ShaderResourceView *> postEffectsSRVs = {
        //     GraphicsManager::Instance().resolved_SRV.Get(), nullptr};

        // GraphicsManager::Instance().SetGlobalConsts(manager->global_consts_GPU);

        //// post effect textures: start from register(20)
        // GraphicsManager::Instance().device_context->PSSetShaderResources(
        //     20, UINT(postEffectsSRVs.size()), postEffectsSRVs.data());
        // //GraphicsManager::Instance().device_context->OMSetRenderTargets(
        // //    1, GraphicsManager::Instance().postEffectsRTV.GetAddressOf(),
        // //    NULL);
        // GraphicsManager::Instance().device_context->OMSetRenderTargets(
        //     1, GraphicsManager::Instance().resolved_RTV.GetAddressOf(),
        //     NULL);

        // GraphicsManager::Instance().device_context->PSSetConstantBuffers(
        //     3, 1, manager->post_effects_consts_GPU.GetAddressOf());

        // if (true) {
        //     Renderer *renderer = nullptr;
        //     manager->screen_square->GetComponent(EnumComponentType::eRenderer,
        //                                           (Component **)(&renderer));
        //     renderer->Render(GraphicsManager::Instance().device_context);
        // }

        GraphicsManager::Instance().SetPipelineState(
            Graphics::postProcessingPSO);
        manager->post_process.Render(GraphicsManager::Instance().device,
                                     GraphicsManager::Instance().device_context,
                                     &manager->global_consts_CPU,
                                     manager->global_consts_GPU);

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace engine

#endif