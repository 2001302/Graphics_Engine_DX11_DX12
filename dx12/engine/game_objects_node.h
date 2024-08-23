#ifndef _RENDERER_DRAW_NODE
#define _RENDERER_DRAW_NODE

#include "../foundation/behavior_tree_builder.h"
#include "black_board.h"
#include "mesh_renderer.h"
// #include "skinned_mesh_renderer.h"

namespace core {

class GameObjectNodeInvoker : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->job_context;
        auto gui = black_board->gui;
        auto command_pool = black_board->command_pool;

        switch (manager->stage_type) {
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
                psoDesc.DepthStencilState.DepthEnable = FALSE;
                psoDesc.DepthStencilState.StencilEnable = FALSE;
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

            // additional object 2
            MeshData mesh = GeometryGenerator::MakeBox(0.2f);
            Vector3 center(0.0f, 0.5f, 2.5f);

            auto renderer = std::make_shared<MeshRenderer>(std::vector{mesh});

            renderer->UpdateWorldRow(Matrix::CreateTranslation(center));
            renderer->material_consts.GetCpu().albedoFactor =
                Vector3(1.0f, 0.2f, 0.2f);
            renderer->material_consts.GetCpu().roughnessFactor = 0.5f;
            renderer->material_consts.GetCpu().metallicFactor = 0.9f;
            renderer->material_consts.GetCpu().emissionFactor = Vector3(0.0f);
            renderer->UpdateConstantBuffers();

            auto obj = std::make_shared<Model>();
            obj->AddComponent(EnumComponentType::eRenderer, renderer);

            manager->objects.insert({obj->GetEntityId(), obj});

            break;
        }
        case EnumStageType::eUpdate: {

            for (auto &i : manager->objects) {
                auto renderer = (MeshRenderer *)i.second->GetComponent(
                    EnumComponentType::eRenderer);
                renderer->UpdateConstantBuffers();
            }

            break;
        }
        case EnumStageType::eRender: {

            ID3D12DescriptorHeap *samplers_heap[] = {
                manager->sampler_heap.Get()};

            for (auto &i : manager->objects) {
                auto renderer = (MeshRenderer *)i.second->GetComponent(
                    EnumComponentType::eRenderer);

                CD3DX12_RECT scissorRect(
                    0, 0,
                    static_cast<LONG>(foundation::Env::Instance().screen_width),
                    static_cast<LONG>(
                        foundation::Env::Instance().screen_height));
                CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
                    dx12::GpuCore::Instance()
                        .rtv_heap->GetCPUDescriptorHandleForHeapStart(),
                    dx12::GpuCore::Instance().frame_index,
                    dx12::GpuCore::Instance().rtv_descriptor_size);
                const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
                command_pool->Get(0)->ClearRenderTargetView(
                    rtvHandle, clearColor, 0, nullptr);

                command_pool->Get(0)->RSSetViewports(
                    1, &dx12::GpuCore::Instance().viewport);
                command_pool->Get(0)->RSSetScissorRects(1, &scissorRect);

                command_pool->Get(0)->OMSetRenderTargets(1, &rtvHandle, false,
                                                         nullptr);

                command_pool->Get(0)->SetGraphicsRootSignature(
                    defaultSolidPSO->root_signature);
                command_pool->Get(0)->SetPipelineState(
                    defaultSolidPSO->pipeline_state);
                command_pool->Get(0)->SetDescriptorHeaps(
                    _countof(samplers_heap), samplers_heap);
                command_pool->Get(0)->SetGraphicsRootDescriptorTable(
                    0, manager->sampler_heap
                           ->GetGPUDescriptorHandleForHeapStart());
                command_pool->Get(0)->SetGraphicsRootConstantBufferView(
                    2, manager->global_consts_GPU->GetGPUVirtualAddress());
                command_pool->Get(0)->SetGraphicsRootConstantBufferView(
                    3, renderer->mesh_consts.Get()->GetGPUVirtualAddress());
                command_pool->Get(0)->SetGraphicsRootConstantBufferView(
                    4, renderer->material_consts.Get()->GetGPUVirtualAddress());

                renderer->Render(command_pool->Get(0));
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
