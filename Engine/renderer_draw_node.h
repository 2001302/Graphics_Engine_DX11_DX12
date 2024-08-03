#ifndef _RENDERER_DRAW_NODE
#define _RENDERER_DRAW_NODE

#include "behavior_tree_builder.h"
#include "renderer.h"
#include "skinned_mesh_renderer.h"
#include "black_board.h"

namespace engine {
class SetSamplerStatesNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;

        GraphicsManager::Instance().SetMainViewport();

        GraphicsManager::Instance().device_context->VSSetSamplers(
            0, UINT(Graphics::sampleStates.size()),
            Graphics::sampleStates.data());
        GraphicsManager::Instance().device_context->PSSetSamplers(
            0, UINT(Graphics::sampleStates.size()),
            Graphics::sampleStates.data());
        GraphicsManager::Instance().device_context->CSSetSamplers(
            0, UINT(Graphics::sampleStates.size()),
            Graphics::sampleStates.data());

        // Shared textures: start from register(t10) in 'Common.hlsli'
        std::vector<ID3D11ShaderResourceView *> commonSRVs = {
            manager->env_SRV.Get(), manager->specular_SRV.Get(),
            manager->irradiance_SRV.Get(), manager->brdf_SRV.Get()};
        GraphicsManager::Instance().device_context->PSSetShaderResources(
            10, UINT(commonSRVs.size()), commonSRVs.data());

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class DrawOnlyDepthNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;

        // Depth Only Pass (no RTS)
        GraphicsManager::Instance().device_context->OMSetRenderTargets(
            0, NULL, GraphicsManager::Instance().m_depthOnlyDSV.Get());
        GraphicsManager::Instance().device_context->ClearDepthStencilView(
            GraphicsManager::Instance().m_depthOnlyDSV.Get(), D3D11_CLEAR_DEPTH,
            1.0f, 0);

        GraphicsManager::Instance().SetPipelineState(Graphics::depthOnlyPSO);
        GraphicsManager::Instance().SetGlobalConsts(manager->global_consts_GPU);

        for (auto &i : manager->models) {
            auto renderer = (Renderer *)i.second->GetComponent(
                EnumComponentType::eRenderer);
            renderer->Render(GraphicsManager::Instance().device_context);
        }

        if (true) {
            auto renderer = (Renderer *)manager->skybox->GetComponent(
                EnumComponentType::eRenderer);
            renderer->Render(GraphicsManager::Instance().device_context);
        }

        if (true) {
            auto renderer = (Renderer *)manager->mirror->GetComponent(
                EnumComponentType::eRenderer);
            renderer->Render(GraphicsManager::Instance().device_context);
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class SetShadowViewportNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        GraphicsManager::Instance().SetShadowViewport(); // shadow map
                                                         // resolution

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class DrawShadowMapNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;

        std::vector<ID3D11ShaderResourceView *> nullSRV(MAX_LIGHTS, nullptr);
        GraphicsManager::Instance().device_context->PSSetShaderResources(
            15, UINT(nullSRV.size()), nullSRV.data());

        // make shadow map
        GraphicsManager::Instance().SetPipelineState(Graphics::depthOnlyPSO);
        for (int i = 0; i < MAX_LIGHTS; i++) {
            if (manager->global_consts_CPU.lights[i].type & LIGHT_SHADOW) {
                // no RTS
                GraphicsManager::Instance().device_context->OMSetRenderTargets(
                    0, NULL, GraphicsManager::Instance().m_shadowDSVs[i].Get());
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
                    auto renderer = (Renderer *)manager->skybox->GetComponent(
                        EnumComponentType::eRenderer);
                    renderer->Render(
                        GraphicsManager::Instance().device_context);
                }

                if (true) {
                    auto renderer = (Renderer *)manager->mirror->GetComponent(
                        EnumComponentType::eRenderer);
                    renderer->Render(
                        GraphicsManager::Instance().device_context);
                }
            }
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class SetMainRenderTargetNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;

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

class DrawObjectsNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;

        GraphicsManager::Instance().SetPipelineState(
            manager->draw_wire ? Graphics::defaultWirePSO
                               : Graphics::defaultSolidPSO);
        GraphicsManager::Instance().SetGlobalConsts(manager->global_consts_GPU);

        for (auto &i : manager->models) {
            auto renderer = (Renderer *)i.second->GetComponent(
                EnumComponentType::eRenderer);
            renderer->Render(GraphicsManager::Instance().device_context);
        }

        // If there is no need to draw mirror reflections, draw only the opaque
        // mirror
        if (manager->mirror_alpha == 1.0f) {
            auto renderer = (Renderer *)manager->mirror->GetComponent(
                EnumComponentType::eRenderer);
            renderer->Render(GraphicsManager::Instance().device_context);
        }

        {
            GraphicsManager::Instance().SetPipelineState(
                manager->draw_wire ? Graphics::skinnedWirePSO
                                   : Graphics::skinnedSolidPSO);
            auto renderer = (SkinnedMeshRenderer *)manager->m_character->GetComponent(
                EnumComponentType::eRenderer);
            renderer->Render(GraphicsManager::Instance().device_context);
        }

        GraphicsManager::Instance().SetPipelineState(Graphics::normalsPSO);
        for (auto &i : manager->models) {
            auto renderer = (Renderer *)i.second->GetComponent(
                EnumComponentType::eRenderer);
            if (renderer->m_drawNormals)
                renderer->RenderNormals(
                    GraphicsManager::Instance().device_context);
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class DrawLightSpheresNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;

        GraphicsManager::Instance().SetPipelineState(
            manager->draw_wire ? Graphics::defaultWirePSO
                               : Graphics::defaultSolidPSO);
        GraphicsManager::Instance().SetGlobalConsts(manager->global_consts_GPU);

        for (auto &i : manager->light_spheres) {
            auto renderer =
                (Renderer *)i->GetComponent(EnumComponentType::eRenderer);
            renderer->Render(GraphicsManager::Instance().device_context);
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class DrawRelatedWithCameraNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;

        GraphicsManager::Instance().SetPipelineState(
            manager->draw_wire ? Graphics::defaultWirePSO
                               : Graphics::defaultSolidPSO);
        GraphicsManager::Instance().SetGlobalConsts(manager->global_consts_GPU);

        manager->camera->Draw();

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class DrawSkyboxNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;

        if (true) {
            GraphicsManager::Instance().SetPipelineState(
                manager->draw_wire ? Graphics::skyboxWirePSO
                                   : Graphics::skyboxSolidPSO);
            auto renderer = (Renderer *)manager->skybox->GetComponent(
                EnumComponentType::eRenderer);
            renderer->Render(GraphicsManager::Instance().device_context);
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class DrawMirrorSurfaceNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;

        // on mirror
        if (manager->mirror_alpha < 1.0f) {

            // Mirror 2. Mark only the mirror position as 1 in the
            // StencilBuffer.
            GraphicsManager::Instance().SetPipelineState(
                Graphics::stencilMaskPSO);

            if (true) {
                auto renderer = (Renderer *)manager->mirror->GetComponent(
                    EnumComponentType::eRenderer);
                renderer->Render(GraphicsManager::Instance().device_context);
            }

            // Mirror 3. Render the reflected objects at the mirror position.
            GraphicsManager::Instance().SetPipelineState(
                manager->draw_wire ? Graphics::reflectWirePSO
                                   : Graphics::reflectSolidPSO);
            GraphicsManager::Instance().SetGlobalConsts(
                manager->reflect_global_consts_GPU);

            GraphicsManager::Instance().device_context->ClearDepthStencilView(
                GraphicsManager::Instance().m_depthStencilView.Get(),
                D3D11_CLEAR_DEPTH, 1.0f, 0);

            for (auto &i : manager->models) {
                auto renderer = (Renderer *)i.second->GetComponent(
                    EnumComponentType::eRenderer);
                renderer->Render(GraphicsManager::Instance().device_context);
            }

            if (true) {
                GraphicsManager::Instance().SetPipelineState(
                    manager->draw_wire ? Graphics::reflectSkyboxWirePSO
                                       : Graphics::reflectSkyboxSolidPSO);
                auto renderer = (Renderer *)manager->skybox->GetComponent(
                    EnumComponentType::eRenderer);
                renderer->Render(GraphicsManager::Instance().device_context);
            }

            if (true) {
                // Mirror 4. Draw the mirror itself with the 'Blend' material
                GraphicsManager::Instance().SetPipelineState(
                    manager->draw_wire ? Graphics::mirrorBlendWirePSO
                                       : Graphics::mirrorBlendSolidPSO);
                GraphicsManager::Instance().SetGlobalConsts(
                    manager->global_consts_GPU);
                auto renderer = (Renderer *)manager->mirror->GetComponent(
                    EnumComponentType::eRenderer);
                renderer->Render(GraphicsManager::Instance().device_context);
            }

        } // end of if (m_mirrorAlpha < 1.0f)
        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class ResolveBufferNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;

        GraphicsManager::Instance().device_context->ResolveSubresource(
            GraphicsManager::Instance().resolved_buffer.Get(), 0,
            GraphicsManager::Instance().float_buffer.Get(), 0,
            DXGI_FORMAT_R16G16B16A16_FLOAT);

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class PresentNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        // Present the rendered scene to the screen.
        if (common::Env::Instance().vsync_enabled) {
            GraphicsManager::Instance().swap_chain->Present(1, 0);
        } else {
            GraphicsManager::Instance().swap_chain->Present(0, 0);
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace engine

#endif