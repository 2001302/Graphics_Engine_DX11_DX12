#ifndef _RENDERER_DRAW_NODE
#define _RENDERER_DRAW_NODE

#include "behavior_tree_builder.h"
#include "rendering_block.h"
#include "renderer.h"
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

        // 공용 텍스춰들: "Common.hlsli"에서 register(t10)부터 시작
        std::vector<ID3D11ShaderResourceView *> commonSRVs = {
            manager->m_envSRV.Get(), manager->m_specularSRV.Get(),
            manager->m_irradianceSRV.Get(), manager->m_brdfSRV.Get()};
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

        // Depth Only Pass (RTS 생략 가능)
        GraphicsManager::Instance().device_context->OMSetRenderTargets(
            0, NULL, GraphicsManager::Instance().m_depthOnlyDSV.Get());
        GraphicsManager::Instance().device_context->ClearDepthStencilView(
            GraphicsManager::Instance().m_depthOnlyDSV.Get(), D3D11_CLEAR_DEPTH,
            1.0f, 0);

        GraphicsManager::Instance().SetPipelineState(Graphics::depthOnlyPSO);
        GraphicsManager::Instance().SetGlobalConsts(manager->m_globalConstsGPU);

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
            manager->m_mirror->GetComponent(EnumComponentType::eRenderer,
                                            (Component **)(&renderer));
            renderer->Render(GraphicsManager::Instance().device_context);
        }

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class SetShadowViewport : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        GraphicsManager::Instance().SetShadowViewport(); // 그림자맵 해상도

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class DrawShadowMap : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        // 그림자맵 만들기
        GraphicsManager::Instance().SetPipelineState(Graphics::depthOnlyPSO);
        for (int i = 0; i < MAX_LIGHTS; i++) {
            if (manager->m_globalConstsCPU.lights[i].type & LIGHT_SHADOW) {
                // RTS 생략 가능
                GraphicsManager::Instance().device_context->OMSetRenderTargets(
                    0, NULL, GraphicsManager::Instance().m_shadowDSVs[i].Get());
                GraphicsManager::Instance()
                    .device_context->ClearDepthStencilView(
                        GraphicsManager::Instance().m_shadowDSVs[i].Get(),
                        D3D11_CLEAR_DEPTH, 1.0f, 0);
                GraphicsManager::Instance().SetGlobalConsts(
                    manager->m_shadowGlobalConstsGPU[i]);

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
                    manager->m_mirror->GetComponent(
                        EnumComponentType::eRenderer,
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

        // 다시 렌더링 해상도로 되돌리기
        GraphicsManager::Instance().SetMainViewport();

        const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        std::vector<ID3D11RenderTargetView *> rtvs = {
            GraphicsManager::Instance().float_RTV.Get()};

        // 거울 1. 거울은 빼고 원래 대로 그리기
        for (size_t i = 0; i < rtvs.size(); i++) {
            GraphicsManager::Instance().device_context->ClearRenderTargetView(
                rtvs[i], clearColor);
        }
        GraphicsManager::Instance().device_context->OMSetRenderTargets(
            UINT(rtvs.size()), rtvs.data(),
            GraphicsManager::Instance().m_depthStencilView.Get());

        // 그림자맵들도 공용 텍스춰들 이후에 추가
        // 주의: 마지막 shadowDSV를 RenderTarget에서 해제한 후 설정
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
            manager->m_drawAsWire ? Graphics::defaultWirePSO
                                  : Graphics::defaultSolidPSO);
        GraphicsManager::Instance().SetGlobalConsts(manager->m_globalConstsGPU);

        for (auto &i : manager->models) {
            Renderer *renderer = nullptr;
            i.second->GetComponent(EnumComponentType::eRenderer,
                            (Component **)(&renderer));
            renderer->Render(GraphicsManager::Instance().device_context);
        }

        // 거울 반사를 그릴 필요가 없으면 불투명 거울만 그리기
        if (manager->m_mirrorAlpha == 1.0f) {

            Renderer *renderer = nullptr;
            manager->m_mirror->GetComponent(EnumComponentType::eRenderer,
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

class DrawSkybox : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        if (true) {
            GraphicsManager::Instance().SetPipelineState(
                manager->m_drawAsWire ? Graphics::skyboxWirePSO
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

        if (manager->m_mirrorAlpha < 1.0f) { // 거울을 그려야 하는 상황

            // 거울 2. 거울 위치만 StencilBuffer에 1로 표기
            GraphicsManager::Instance().SetPipelineState(
                Graphics::stencilMaskPSO);

            if (true) {
                Renderer *renderer = nullptr;
                manager->m_mirror->GetComponent(EnumComponentType::eRenderer,
                                                (Component **)(&renderer));
                renderer->Render(GraphicsManager::Instance().device_context);
            }

            // 거울 3. 거울 위치에 반사된 물체들을 렌더링
            GraphicsManager::Instance().SetPipelineState(
                manager->m_drawAsWire ? Graphics::reflectWirePSO
                                      : Graphics::reflectSolidPSO);
            GraphicsManager::Instance().SetGlobalConsts(
                manager->m_reflectGlobalConstsGPU);

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
                    manager->m_drawAsWire ? Graphics::reflectSkyboxWirePSO
                                          : Graphics::reflectSkyboxSolidPSO);
                Renderer *renderer = nullptr;
                manager->skybox->GetComponent(EnumComponentType::eRenderer,
                                              (Component **)(&renderer));
                renderer->Render(GraphicsManager::Instance().device_context);
            }

            if (true) {
                // 거울 4. 거울 자체의 재질을 "Blend"로 그림
                GraphicsManager::Instance().SetPipelineState(
                    manager->m_drawAsWire ? Graphics::mirrorBlendWirePSO
                                          : Graphics::mirrorBlendSolidPSO);
                GraphicsManager::Instance().SetGlobalConsts(
                    manager->m_globalConstsGPU);
                Renderer *renderer = nullptr;
                manager->m_mirror->GetComponent(EnumComponentType::eRenderer,
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

        // PostEffects
        GraphicsManager::Instance().SetPipelineState(Graphics::postEffectsPSO);

        std::vector<ID3D11ShaderResourceView *> postEffectsSRVs = {
            GraphicsManager::Instance().resolved_SRV.Get(), nullptr};

        // 그림자맵 확인용 임시
        // AppBase::SetGlobalConsts(m_shadowGlobalConstsGPU[0]);
        GraphicsManager::Instance().SetGlobalConsts(manager->m_globalConstsGPU);
        // vector<ID3D11ShaderResourceView *> postEffectsSRVs = {
        //  m_resolvedSRV.Get(), m_shadowSRVs[1].Get()};

        // 20번에 넣어줌
        GraphicsManager::Instance().device_context->PSSetShaderResources(
            20, UINT(postEffectsSRVs.size()), postEffectsSRVs.data());
        GraphicsManager::Instance().device_context->OMSetRenderTargets(
            1, GraphicsManager::Instance().postEffectsRTV.GetAddressOf(), NULL);
        // m_GraphicsManager::Instance().device_context->OMSetRenderTargets(1,
        // m_backBufferRTV.GetAddressOf(), NULL);

        GraphicsManager::Instance().device_context->PSSetConstantBuffers(
            3, 1, manager->m_postEffectsConstsGPU.GetAddressOf());

        if (true) {
            Renderer *renderer = nullptr;
            manager->screen_square->GetComponent(EnumComponentType::eRenderer,
                                                  (Component **)(&renderer));
            renderer->Render(GraphicsManager::Instance().device_context);
        }

        GraphicsManager::Instance().SetPipelineState(
            Graphics::postProcessingPSO);
        manager->m_postProcess.Render(
            GraphicsManager::Instance().device_context);

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

        gui->FrameBegin();
        gui->FrameRate();
        gui->StyleSetting();
        gui->MenuBar();
        gui->NodeEditor();
        gui->TabBar(manager->models);
        gui->FrameEnd();
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