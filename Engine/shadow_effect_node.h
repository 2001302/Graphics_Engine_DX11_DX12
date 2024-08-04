#ifndef _SHADOW_EFFECT
#define _SHADOW_EFFECT

#include "behavior_tree_builder.h"
#include "black_board.h"
#include "mesh_renderer.h"

namespace engine {
class ShadowEffectNodeInvoker : public common::BehaviorActionNode {

    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block.get();

        switch (manager->stage_type) {
        case EnumStageType::eInitialize: {

            D3D11_TEXTURE2D_DESC desc;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.CPUAccessFlags = 0;
            desc.MiscFlags = 0;
            desc.Format = DXGI_FORMAT_R32_TYPELESS;
            desc.BindFlags =
                D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;

            // shadow buffers (depth only)
            desc.Width = m_shadowWidth;
            desc.Height = m_shadowHeight;
            for (int i = 0; i < MAX_LIGHTS; i++) {
                ThrowIfFailed(
                    GraphicsManager::Instance().device->CreateTexture2D(
                        &desc, NULL, m_shadowBuffers[i].GetAddressOf()));
            }

            D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
            ZeroMemory(&dsvDesc, sizeof(dsvDesc));
            dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
            dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

            // shadow DSVs
            for (int i = 0; i < MAX_LIGHTS; i++) {
                ThrowIfFailed(
                    GraphicsManager::Instance().device->CreateDepthStencilView(
                        m_shadowBuffers[i].Get(), &dsvDesc,
                        m_shadowDSVs[i].GetAddressOf()));
            }

            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
            ZeroMemory(&srvDesc, sizeof(srvDesc));
            srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;

            // shadow SRVs
            for (int i = 0; i < MAX_LIGHTS; i++) {
                ThrowIfFailed(GraphicsManager::Instance()
                                  .device->CreateShaderResourceView(
                                      m_shadowBuffers[i].Get(), &srvDesc,
                                      m_shadowSRVs[i].GetAddressOf()));
            }
            
            // shadow global constant buffer
            for (int i = 0; i < MAX_LIGHTS; i++) {
                GraphicsUtil::CreateConstBuffer(
                    GraphicsManager::Instance().device,
                    shadow_global_consts_CPU[i],
                    shadow_global_consts_GPU[i]);
            }

            break;
        }
        case EnumStageType::eUpdate: {

            // viewpoint for creating the shadow map
            for (int i = 0; i < MAX_LIGHTS; i++) {
                const auto &light = manager->global_consts_CPU.lights[i];
                if (light.type & LIGHT_SHADOW) {

                    Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
                    if (abs(up.Dot(light.direction) + 1.0f) < 1e-5)
                        up = Vector3(1.0f, 0.0f, 0.0f);

                    Matrix lightViewRow = DirectX::XMMatrixLookAtLH(
                        light.position, light.position + light.direction, up);

                    Matrix lightProjRow = DirectX::XMMatrixPerspectiveFovLH(
                        DirectX::XMConvertToRadians(120.0f), 1.0f, 0.1f, 10.0f);

                    shadow_global_consts_CPU[i].eyeWorld = light.position;
                    shadow_global_consts_CPU[i].view = lightViewRow.Transpose();
                    shadow_global_consts_CPU[i].proj = lightProjRow.Transpose();
                    shadow_global_consts_CPU[i].invProj =
                        lightProjRow.Invert().Transpose();
                    shadow_global_consts_CPU[i].viewProj =
                        (lightViewRow * lightProjRow).Transpose();

                    GraphicsUtil::UpdateBuffer(
                        GraphicsManager::Instance().device_context,
                        shadow_global_consts_CPU[i],
                        shadow_global_consts_GPU[i]);

                    manager->global_consts_CPU.lights[i].viewProj =
                        shadow_global_consts_CPU[i].viewProj;
                    manager->global_consts_CPU.lights[i].invProj =
                        shadow_global_consts_CPU[i].invProj;
                }
            }
            break;
        }
        case EnumStageType::eRender: {

            SetShadowViewport(GraphicsManager::Instance().device_context);

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
                            0, NULL, m_shadowDSVs[i].Get());
                    GraphicsManager::Instance()
                        .device_context->ClearDepthStencilView(
                            m_shadowDSVs[i].Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
                    GraphicsManager::Instance().SetGlobalConsts(
                        shadow_global_consts_GPU[i]);

                    for (auto &i : manager->models) {
                        auto renderer = (MeshRenderer *)i.second->GetComponent(
                            EnumComponentType::eRenderer);
                        if (renderer->m_castShadow && renderer->m_isVisible)
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
            shadowSRVs.push_back(m_shadowSRVs[i].Get());
        }
        GraphicsManager::Instance().device_context->PSSetShaderResources(
            15, UINT(shadowSRVs.size()), shadowSRVs.data());

        GraphicsManager::Instance().device_context->ClearDepthStencilView(
            GraphicsManager::Instance().m_depthStencilView.Get(),
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
    void SetShadowViewport(ComPtr<ID3D11DeviceContext> context) {

        // Set the viewport
        D3D11_VIEWPORT shadowViewport;
        ZeroMemory(&shadowViewport, sizeof(D3D11_VIEWPORT));
        shadowViewport.TopLeftX = 0;
        shadowViewport.TopLeftY = 0;
        shadowViewport.Width = float(m_shadowWidth);
        shadowViewport.Height = float(m_shadowHeight);
        shadowViewport.MinDepth = 0.0f;
        shadowViewport.MaxDepth = 1.0f;

        context->RSSetViewports(1, &shadowViewport);
    }

    int m_shadowWidth = 1280;
    int m_shadowHeight = 1280;
    ComPtr<ID3D11Texture2D> m_shadowBuffers[MAX_LIGHTS]; // No MSAA
    ComPtr<ID3D11DepthStencilView> m_shadowDSVs[MAX_LIGHTS];
    ComPtr<ID3D11ShaderResourceView> m_shadowSRVs[MAX_LIGHTS];

    GlobalConstants shadow_global_consts_CPU[MAX_LIGHTS];
    ComPtr<ID3D11Buffer> shadow_global_consts_GPU[MAX_LIGHTS];
};

} // namespace engine

#endif