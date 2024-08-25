#ifndef _RENDERER_DRAW_NODE
#define _RENDERER_DRAW_NODE

#include "../foundation/behavior_tree_builder.h"
#include "../graphics/command_pool.h"
#include "black_board.h"
#include "mesh_renderer.h"

namespace core {

class GameObjectNodeInvoker : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto condition = black_board->conditions.get();
        auto targets = black_board->targets.get();

        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {

            { // pso
                defaultSolidPSO = std::make_shared<dx12::GraphicsPSO>();
                // layout
                D3D12_INPUT_ELEMENT_DESC basicIEs[] = {
                    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
                     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
                     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
                     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32,
                     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                };
                // shader
                ComPtr<ID3DBlob> basicVS;
                ComPtr<ID3DBlob> basicPS;
                dx12::Util::CreateVertexShader(dx12::GpuCore::Instance().device,
                                               L"graphics/BasicVS.hlsl",
                                               basicVS);
                dx12::Util::CreatePixelShader(dx12::GpuCore::Instance().device,
                                              L"graphics/BasicPS.hlsl",
                                              basicPS);
                // rasterizer
                D3D12_RASTERIZER_DESC solidRS;
                ZeroMemory(&solidRS, sizeof(D3D12_RASTERIZER_DESC));
                solidRS.FillMode = D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
                solidRS.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
                solidRS.FrontCounterClockwise = false;
                solidRS.DepthClipEnable = true;
                solidRS.MultisampleEnable = true;

                // depth stencil
                D3D12_DEPTH_STENCIL_DESC dsDesc;
                ZeroMemory(&dsDesc, sizeof(dsDesc));
                dsDesc.DepthEnable = true;
                dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
                dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
                dsDesc.StencilEnable = false; // Stencil 불필요
                dsDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
                dsDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
                // 앞면에 대해서 어떻게 작동할지 설정
                dsDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
                dsDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
                dsDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
                dsDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
                // 뒷면에 대해 어떻게 작동할지 설정 (뒷면도 그릴 경우)
                dsDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
                dsDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
                dsDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
                dsDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

                // s0 ~ s6
                CD3DX12_DESCRIPTOR_RANGE1 samplerRange;
                samplerRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 6, 0);

                // t10 ~ t16
                CD3DX12_DESCRIPTOR_RANGE1 textureRange;
                textureRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 10);

                // rootSignature
                CD3DX12_ROOT_PARAMETER1 rootParameters[7] = {};
                // Common.hlsli : s0~s6,t10~t16,b0~b2
                rootParameters[0].InitAsDescriptorTable(
                    1, &samplerRange, D3D12_SHADER_VISIBILITY_ALL);
                rootParameters[1].InitAsDescriptorTable(
                    1, &textureRange, D3D12_SHADER_VISIBILITY_ALL);
                rootParameters[2].InitAsConstantBufferView(
                    0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
                    D3D12_SHADER_VISIBILITY_ALL);
                rootParameters[3].InitAsConstantBufferView(
                    1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
                    D3D12_SHADER_VISIBILITY_ALL);
                rootParameters[4].InitAsConstantBufferView(
                    2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
                    D3D12_SHADER_VISIBILITY_ALL);
                // VS : t0
                CD3DX12_DESCRIPTOR_RANGE1 textureRangeVS;
                textureRangeVS.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
                rootParameters[5].InitAsDescriptorTable(
                    1, &textureRangeVS, D3D12_SHADER_VISIBILITY_VERTEX);
                // PS : t0 ~ t4
                CD3DX12_DESCRIPTOR_RANGE1 textureRangePS;
                textureRangePS.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0);
                rootParameters[6].InitAsDescriptorTable(
                    1, &textureRangePS, D3D12_SHADER_VISIBILITY_PIXEL);

                auto rootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(
                    ARRAYSIZE(rootParameters), rootParameters, 0, nullptr,
                    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

                ComPtr<ID3DBlob> signature;
                ComPtr<ID3DBlob> error;
                HRESULT hr = D3D12SerializeVersionedRootSignature(
                    &rootSignatureDesc, &signature, &error);

                hr = dx12::GpuCore::Instance().device->CreateRootSignature(
                    0, signature->GetBufferPointer(),
                    signature->GetBufferSize(),
                    IID_PPV_ARGS(&defaultSolidPSO->root_signature));

                // defaultSolidPSO;
                D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
                psoDesc.InputLayout = {basicIEs, _countof(basicIEs)};
                psoDesc.pRootSignature = defaultSolidPSO->root_signature;
                psoDesc.VS = CD3DX12_SHADER_BYTECODE(basicVS.Get());
                psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
                psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(solidRS);
                psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
                psoDesc.DepthStencilState = dsDesc;
                psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
                psoDesc.SampleMask = UINT_MAX;
                psoDesc.PrimitiveTopologyType =
                    D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
                psoDesc.NumRenderTargets = 1;
                psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
                psoDesc.SampleDesc.Count = 1;

                dx12::ThrowIfFailed(
                    dx12::GpuCore::Instance()
                        .device->CreateGraphicsPipelineState(
                            &psoDesc,
                            IID_PPV_ARGS(&defaultSolidPSO->pipeline_state)));
            }

            // sample object
            Vector3 center(0.0f, 0.0f, 0.0f);
            std::string base_path = "Assets/Characters/zelda/";
            std::string file_name = "zeldaPosed001.fbx";

            auto renderer =
                std::make_shared<MeshRenderer>(base_path, file_name);

            renderer->UpdateWorldRow(Matrix::CreateTranslation(center));
            renderer->UpdateConstantBuffers();

            auto obj = std::make_shared<Model>();
            obj->AddComponent(EnumComponentType::eRenderer, renderer);

            targets->objects.insert({obj->GetEntityId(), obj});

            break;
        }
        case EnumStageType::eUpdate: {
            for (auto &i : targets->objects) {
                auto renderer = (MeshRenderer *)i.second->GetComponent(
                    EnumComponentType::eRenderer);
                renderer->UpdateConstantBuffers();
            }

            break;
        }
        case EnumStageType::eRender: {
            for (auto &i : targets->objects) {
                auto renderer = (MeshRenderer *)i.second->GetComponent(
                    EnumComponentType::eRenderer);
                renderer->Render(condition, defaultSolidPSO.get());
            }

            break;
        }
        default:
            break;
        }

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }

    std::shared_ptr<dx12::GraphicsPSO> defaultSolidPSO;
    std::shared_ptr<dx12::GraphicsPSO> defaultWirePSO;
};
} // namespace core

#endif
