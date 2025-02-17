#ifndef _SHADOW_EFFECT
#define _SHADOW_EFFECT

#include "../pipeline/black_board.h"
#include "../component/mesh_renderer.h"
#include <behaviortree/behavior_tree_builder.h>

namespace graphics {
class ShadowEffectNodeInvoker : public common::BehaviorActionNode {

    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->targets.get();

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
            desc.Width = shadow_width;
            desc.Height = shadow_height;
            for (int i = 0; i < MAX_LIGHTS; i++) {
                ASSERT_FAILED(GpuCore::Instance().device->CreateTexture2D(
                    &desc, NULL, shadow_buffers[i].GetAddressOf()));
            }

            D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
            ZeroMemory(&dsvDesc, sizeof(dsvDesc));
            dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
            dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

            // shadow DSVs
            for (int i = 0; i < MAX_LIGHTS; i++) {
                ASSERT_FAILED(
                    GpuCore::Instance().device->CreateDepthStencilView(
                        shadow_buffers[i].Get(), &dsvDesc,
                        shadow_DSVs[i].GetAddressOf()));
            }

            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
            ZeroMemory(&srvDesc, sizeof(srvDesc));
            srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;

            // shadow SRVs
            for (int i = 0; i < MAX_LIGHTS; i++) {
                ASSERT_FAILED(
                    GpuCore::Instance().device->CreateShaderResourceView(
                        shadow_buffers[i].Get(), &srvDesc,
                        shadow_SRVs[i].GetAddressOf()));
            }

            // shadow global constant buffer
            for (int i = 0; i < MAX_LIGHTS; i++) {
                Util::CreateConstBuffer(shadow_global_consts_CPU[i],
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

                    Util::UpdateBuffer(shadow_global_consts_CPU[i],
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

            SetShadowViewport();

            std::vector<ID3D11ShaderResourceView *> nullSRV(MAX_LIGHTS,
                                                            nullptr);
            GpuCore::Instance().device_context->PSSetShaderResources(
                15, UINT(nullSRV.size()), nullSRV.data());

            // make shadow map
            Util::SetPipelineState(pipeline::depthOnlyPSO);
            for (int i = 0; i < MAX_LIGHTS; i++) {
                if (manager->global_consts_CPU.lights[i].type & LIGHT_SHADOW) {
                    // no RTS
                    GpuCore::Instance().device_context->OMSetRenderTargets(
                        0, NULL, shadow_DSVs[i].Get());
                    GpuCore::Instance().device_context->ClearDepthStencilView(
                        shadow_DSVs[i].Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
                    Util::SetGlobalConsts(shadow_global_consts_GPU[i]);

                    for (auto &i : manager->objects) {
                        MeshRenderer *renderer = nullptr;
                        if (i.second->TryGet(renderer)) {
							if (renderer->cast_shadow && renderer->is_visible)
								renderer->Render();
						}
                    }
                }
            }

            // rendering resolution
            GpuCore::Instance().SetMainViewport();

            const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
            std::vector<ID3D11RenderTargetView *> rtvs = {
                GpuCore::Instance().float_RTV.Get()};

            // Mirror 1. Draw it as it originally is, without the mirror.
            for (size_t i = 0; i < rtvs.size(); i++) {
                GpuCore::Instance().device_context->ClearRenderTargetView(
                    rtvs[i], clearColor);
            }
            GpuCore::Instance().device_context->OMSetRenderTargets(
                UINT(rtvs.size()), rtvs.data(),
                GpuCore::Instance().depth_stencil_view.Get());

            // Shadow textures: start from register(15)
            // Note: Unbind the last shadowDSV from the RenderTarget before
            // setting it.
            std::vector<ID3D11ShaderResourceView *> shadowSRVs;
            for (int i = 0; i < MAX_LIGHTS; i++) {
                shadowSRVs.push_back(shadow_SRVs[i].Get());
            }
            GpuCore::Instance().device_context->PSSetShaderResources(
                15, UINT(shadowSRVs.size()), shadowSRVs.data());

            GpuCore::Instance().device_context->ClearDepthStencilView(
                GpuCore::Instance().depth_stencil_view.Get(),
                D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
            break;
        }
        default:
            break;
        }


        return common::EnumBehaviorTreeStatus::eSuccess;
    }
    void SetShadowViewport() {

        // Set the viewport
        D3D11_VIEWPORT shadowViewport;
        ZeroMemory(&shadowViewport, sizeof(D3D11_VIEWPORT));
        shadowViewport.TopLeftX = 0;
        shadowViewport.TopLeftY = 0;
        shadowViewport.Width = float(shadow_width);
        shadowViewport.Height = float(shadow_height);
        shadowViewport.MinDepth = 0.0f;
        shadowViewport.MaxDepth = 1.0f;

        GpuCore::Instance().device_context->RSSetViewports(1, &shadowViewport);
    }

    int shadow_width = 1280;
    int shadow_height = 1280;
    ComPtr<ID3D11Texture2D> shadow_buffers[MAX_LIGHTS]; // No MSAA
    ComPtr<ID3D11DepthStencilView> shadow_DSVs[MAX_LIGHTS];
    ComPtr<ID3D11ShaderResourceView> shadow_SRVs[MAX_LIGHTS];

    GlobalConstants shadow_global_consts_CPU[MAX_LIGHTS];
    ComPtr<ID3D11Buffer> shadow_global_consts_GPU[MAX_LIGHTS];
};

} // namespace graphics

#endif