#ifndef _RENDERER_DRAW_NODE
#define _RENDERER_DRAW_NODE

#include "behavior_tree_builder.h"
#include "renderer.h"
#include "rendering_block.h"
#include "setting_ui.h"

namespace engine {
class SetSamplerStates : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

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

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class DrawOnlyDepth : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        // Depth Only Pass (no RTS)
        GraphicsManager::Instance().device_context->OMSetRenderTargets(
            0, NULL, GraphicsManager::Instance().m_depthOnlyDSV.Get());
        GraphicsManager::Instance().device_context->ClearDepthStencilView(
            GraphicsManager::Instance().m_depthOnlyDSV.Get(), D3D11_CLEAR_DEPTH,
            1.0f, 0);

        GraphicsManager::Instance().SetPipelineState(Graphics::depthOnlyPSO);
        GraphicsManager::Instance().SetGlobalConsts(manager->global_consts_GPU);

        for (auto &i : manager->models) {
            Renderer *renderer = nullptr;
            i.second->GetComponent(EnumComponentType::eRenderer,
                                   (Component **)(&renderer));
            renderer->Render(GraphicsManager::Instance().device_context);
        }

        if (true) {
            Renderer *renderer = nullptr;
            manager->skybox->GetComponent(EnumComponentType::eRenderer,
                                          (Component **)(&renderer));
            renderer->Render(GraphicsManager::Instance().device_context);
        }

        if (true) {
            Renderer *renderer = nullptr;
            manager->mirror->GetComponent(EnumComponentType::eRenderer,
                                          (Component **)(&renderer));
            renderer->Render(GraphicsManager::Instance().device_context);
        }

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class SetShadowViewport : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        GraphicsManager::Instance().SetShadowViewport(); // shadow map
                                                         // resolution

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class DrawShadowMap : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

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
                    Renderer *renderer = nullptr;
                    i.second->GetComponent(EnumComponentType::eRenderer,
                                           (Component **)(&renderer));

                    if (renderer->m_castShadow && renderer->m_isVisible)
                        renderer->Render(
                            GraphicsManager::Instance().device_context);
                }

                if (true) {
                    Renderer *renderer = nullptr;
                    manager->skybox->GetComponent(EnumComponentType::eRenderer,
                                                  (Component **)(&renderer));
                    renderer->Render(
                        GraphicsManager::Instance().device_context);
                }

                if (true) {
                    Renderer *renderer = nullptr;
                    manager->mirror->GetComponent(EnumComponentType::eRenderer,
                                                  (Component **)(&renderer));
                    renderer->Render(
                        GraphicsManager::Instance().device_context);
                }
            }
        }

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class SetMainRenderTarget : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

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

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class DrawObjects : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        GraphicsManager::Instance().SetPipelineState(
            manager->draw_wire ? Graphics::defaultWirePSO
                               : Graphics::defaultSolidPSO);
        GraphicsManager::Instance().SetGlobalConsts(manager->global_consts_GPU);

        for (auto &i : manager->models) {
            Renderer *renderer = nullptr;
            i.second->GetComponent(EnumComponentType::eRenderer,
                                   (Component **)(&renderer));
            renderer->Render(GraphicsManager::Instance().device_context);
        }

        // If there is no need to draw mirror reflections, draw only the opaque
        // mirror
        if (manager->mirror_alpha == 1.0f) {

            Renderer *renderer = nullptr;
            manager->mirror->GetComponent(EnumComponentType::eRenderer,
                                          (Component **)(&renderer));
            renderer->Render(GraphicsManager::Instance().device_context);
        }

        GraphicsManager::Instance().SetPipelineState(Graphics::normalsPSO);
        for (auto &i : manager->models) {

            Renderer *renderer = nullptr;
            i.second->GetComponent(EnumComponentType::eRenderer,
                                   (Component **)(&renderer));
            if (renderer->m_drawNormals)
                renderer->RenderNormals(
                    GraphicsManager::Instance().device_context);
        }

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class DrawLightSpheres : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        GraphicsManager::Instance().SetPipelineState(
            manager->draw_wire ? Graphics::defaultWirePSO
                               : Graphics::defaultSolidPSO);
        GraphicsManager::Instance().SetGlobalConsts(manager->global_consts_GPU);

        for (auto &i : manager->light_spheres) {
            Renderer *renderer = nullptr;
            i->GetComponent(EnumComponentType::eRenderer,
                            (Component **)(&renderer));
            renderer->Render(GraphicsManager::Instance().device_context);
        }

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class DrawRelatedWithCamera : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        GraphicsManager::Instance().SetPipelineState(
            manager->draw_wire ? Graphics::defaultWirePSO
                               : Graphics::defaultSolidPSO);
        GraphicsManager::Instance().SetGlobalConsts(manager->global_consts_GPU);

        manager->camera->Draw();

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class DrawSkybox : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        if (true) {
            GraphicsManager::Instance().SetPipelineState(
                manager->draw_wire ? Graphics::skyboxWirePSO
                                   : Graphics::skyboxSolidPSO);
            Renderer *renderer = nullptr;
            manager->skybox->GetComponent(EnumComponentType::eRenderer,
                                          (Component **)(&renderer));
            renderer->Render(GraphicsManager::Instance().device_context);
        }

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class DrawMirrorSurface : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        // on mirror
        if (manager->mirror_alpha < 1.0f) {

            // Mirror 2. Mark only the mirror position as 1 in the
            // StencilBuffer.
            GraphicsManager::Instance().SetPipelineState(
                Graphics::stencilMaskPSO);

            if (true) {
                Renderer *renderer = nullptr;
                manager->mirror->GetComponent(EnumComponentType::eRenderer,
                                              (Component **)(&renderer));
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
                Renderer *renderer = nullptr;
                i.second->GetComponent(EnumComponentType::eRenderer,
                                       (Component **)(&renderer));
                renderer->Render(GraphicsManager::Instance().device_context);
            }

            if (true) {
                GraphicsManager::Instance().SetPipelineState(
                    manager->draw_wire ? Graphics::reflectSkyboxWirePSO
                                       : Graphics::reflectSkyboxSolidPSO);
                Renderer *renderer = nullptr;
                manager->skybox->GetComponent(EnumComponentType::eRenderer,
                                              (Component **)(&renderer));
                renderer->Render(GraphicsManager::Instance().device_context);
            }

            if (true) {
                // Mirror 4. Draw the mirror itself with the 'Blend' material
                GraphicsManager::Instance().SetPipelineState(
                    manager->draw_wire ? Graphics::mirrorBlendWirePSO
                                       : Graphics::mirrorBlendSolidPSO);
                GraphicsManager::Instance().SetGlobalConsts(
                    manager->global_consts_GPU);
                Renderer *renderer = nullptr;
                manager->mirror->GetComponent(EnumComponentType::eRenderer,
                                              (Component **)(&renderer));
                renderer->Render(GraphicsManager::Instance().device_context);
            }

        } // end of if (m_mirrorAlpha < 1.0f)
        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class ResolveBuffer : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        GraphicsManager::Instance().device_context->ResolveSubresource(
            GraphicsManager::Instance().resolved_buffer.Get(), 0,
            GraphicsManager::Instance().float_buffer.Get(), 0,
            DXGI_FORMAT_R16G16B16A16_FLOAT);

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class DrawPostProcessing : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
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
        //     1, GraphicsManager::Instance().resolved_RTV.GetAddressOf(), NULL);

        // GraphicsManager::Instance().device_context->PSSetConstantBuffers(
        //     3, 1, manager->post_effects_consts_GPU.GetAddressOf());

         //if (true) {
         //    Renderer *renderer = nullptr;
         //    manager->screen_square->GetComponent(EnumComponentType::eRenderer,
         //                                          (Component **)(&renderer));
         //    renderer->Render(GraphicsManager::Instance().device_context);
         //}

        GraphicsManager::Instance().SetPipelineState(
            Graphics::postProcessingPSO);
        manager->post_process.Render(GraphicsManager::Instance().device,
                                     GraphicsManager::Instance().device_context,
                                     &manager->global_consts_CPU,
                                     manager->global_consts_GPU);

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class DrawSettingUi : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        auto gui = dynamic_cast<common::SettingUi *>(
            data_block[EnumDataBlockType::eGui]);
        assert(gui != nullptr);

        gui->PushNode(dynamic_cast<common::INode *>(manager));
        gui->Frame(manager);
        gui->ClearNode();

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class Present : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        // Present the rendered scene to the screen.
        if (common::Env::Instance().vsync_enabled) {
            GraphicsManager::Instance().swap_chain->Present(1, 0);
        } else {
            GraphicsManager::Instance().swap_chain->Present(0, 0);
        }

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace engine

#endif