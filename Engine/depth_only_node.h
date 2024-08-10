#ifndef _DEPTH_ONLY_NODE
#define _DEPTH_ONLY_NODE

#include "behavior_tree_builder.h"
#include "black_board.h"
#include "graphics_manager.h"

namespace engine {
class OnlyDepthNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->job_context;

        switch (manager->stage_type) {
        case EnumStageType::eInitialize: {

            D3D11_TEXTURE2D_DESC desc;
            desc.Width = common::Env::Instance().screen_width;
            desc.Height = common::Env::Instance().screen_height;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.CPUAccessFlags = 0;
            desc.MiscFlags = 0;

            // Depth 전용
            desc.Format = DXGI_FORMAT_R32_TYPELESS;
            desc.BindFlags =
                D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            ThrowIfFailed(GraphicsManager::Instance().device->CreateTexture2D(
                &desc, NULL, depthOnlyBuffer.GetAddressOf()));
            break;
        }
        case EnumStageType::eRender: {
            // Depth Only Pass (no RTS)
            GraphicsManager::Instance().device_context->OMSetRenderTargets(
                0, NULL, depthOnlyDSV.Get());
            GraphicsManager::Instance().device_context->ClearDepthStencilView(
                depthOnlyDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

            GraphicsManager::Instance().SetPipelineState(
                Graphics::depthOnlyPSO);
            GraphicsManager::Instance().SetGlobalConsts(
                manager->global_consts_GPU);

            for (auto &i : manager->models) {
                auto renderer = (MeshRenderer *)i.second->GetComponent(
                    EnumComponentType::eRenderer);
                renderer->Render(GraphicsManager::Instance().device_context);
            }

            if (true) {
                auto renderer = (MeshRenderer *)manager->skybox->GetComponent(
                    EnumComponentType::eRenderer);
                renderer->Render(GraphicsManager::Instance().device_context);
            }

            if (true) {
                auto renderer = (MeshRenderer *)manager->mirror->GetComponent(
                    EnumComponentType::eRenderer);
                renderer->Render(GraphicsManager::Instance().device_context);
            }
            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }

    ComPtr<ID3D11Texture2D> depthOnlyBuffer; // No MSAA
    ComPtr<ID3D11DepthStencilView> depthOnlyDSV;
    ComPtr<ID3D11ShaderResourceView> depthOnlySRV;
};
} // namespace engine

#endif