#ifndef _SHADOW_EFFECT
#define _SHADOW_EFFECT

#include "behavior_tree_builder.h"
#include "black_board.h"
#include "renderer.h"

namespace engine {

class ShadowEffectNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block.get();

        GraphicsManager::Instance().SetShadowViewport(); // shadow map
                                                         // resolution
        switch (manager->stage_type) {
        case EnumStageType::eRender: {

            std::vector<ID3D11ShaderResourceView *> nullSRV(MAX_LIGHTS,
                                                            nullptr);
            GraphicsManager::Instance().device_context->PSSetShaderResources(
                15, UINT(nullSRV.size()), nullSRV.data());

            // make shadow map
            GraphicsManager::Instance().SetPipelineState(
                Graphics::depthOnlyPSO);
            for (int i = 0; i < MAX_LIGHTS; i++) {
                if (manager->global_consts_CPU.lights[i].type & LIGHT_SHADOW) {
                    // no RTS
                    GraphicsManager::Instance()
                        .device_context->OMSetRenderTargets(
                            0, NULL,
                            GraphicsManager::Instance().m_shadowDSVs[i].Get());
                    GraphicsManager::Instance()
                        .device_context->ClearDepthStencilView(
                            GraphicsManager::Instance().m_shadowDSVs[i].Get(),
                            D3D11_CLEAR_DEPTH, 1.0f, 0);
                    GraphicsManager::Instance().SetGlobalConsts(
                        manager->shadow_global_consts_GPU[i]);

                    for (auto &i : manager->models) {
                        auto renderer = (Renderer *)i.second->GetComponent(
                            EnumComponentType::eRenderer);
                        if (renderer->m_castShadow && renderer->m_isVisible)
                            renderer->Render(
                                GraphicsManager::Instance().device_context);
                    }

                    if (true) {
                        auto renderer =
                            (Renderer *)manager->skybox->GetComponent(
                                EnumComponentType::eRenderer);
                        renderer->Render(
                            GraphicsManager::Instance().device_context);
                    }

                    if (true) {
                        auto renderer =
                            (Renderer *)manager->mirror->GetComponent(
                                EnumComponentType::eRenderer);
                        renderer->Render(
                            GraphicsManager::Instance().device_context);
                    }
                }
            }
            break;
        }
        default:
            break;
        }

        // rendering resolution
        GraphicsManager::Instance().SetMainViewport();

        const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        std::vector<ID3D11RenderTargetView *> rtvs = {
            GraphicsManager::Instance().float_RTV.Get()};

        // Mirror 1. Draw it as it originally is, without the mirror.
        for (size_t i = 0; i < rtvs.size(); i++) {
            GraphicsManager::Instance().device_context->ClearRenderTargetView(
                rtvs[i], clearColor);
        }
        GraphicsManager::Instance().device_context->OMSetRenderTargets(
            UINT(rtvs.size()), rtvs.data(),
            GraphicsManager::Instance().m_depthStencilView.Get());

        // Shadow textures: start from register(15)
        // Note: Unbind the last shadowDSV from the RenderTarget before setting
        // it.
        std::vector<ID3D11ShaderResourceView *> shadowSRVs;
        for (int i = 0; i < MAX_LIGHTS; i++) {
            shadowSRVs.push_back(
                GraphicsManager::Instance().m_shadowSRVs[i].Get());
        }
        GraphicsManager::Instance().device_context->PSSetShaderResources(
            15, UINT(shadowSRVs.size()), shadowSRVs.data());

        GraphicsManager::Instance().device_context->ClearDepthStencilView(
            GraphicsManager::Instance().m_depthStencilView.Get(),
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace engine

#endif