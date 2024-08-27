#ifndef _TONE_MAPPING_NODE
#define _TONE_MAPPING_NODE

#include "../foundation/behavior_tree_builder.h"
#include "../graphics/command_pool.h"
#include "black_board.h"
#include "mesh_renderer.h"

namespace core {

class ToneMappingNodeInvoker : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto condition = black_board->conditions.get();
        auto targets = black_board->targets.get();

        auto command_list = condition->command_pool->Get(0);
        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {

            { // pso
                toneMappingPSO = std::make_shared<dx12::GraphicsPSO>();
                auto mesh_data = GeometryGenerator::MakeSquare();
                mesh = std::make_shared<Mesh>();

                dx12::Util::CreateVertexBuffer(mesh_data.vertices,
                                               mesh->vertex_buffer,
                                               mesh->vertex_buffer_view);
                mesh->index_count = UINT(mesh_data.indices.size());
                mesh->vertex_count = UINT(mesh_data.vertices.size());
                mesh->stride = UINT(sizeof(Vertex));
                dx12::Util::CreateIndexBuffer(mesh_data.indices,
                                              mesh->index_buffer,
                                              mesh->index_buffer_view);

                CD3DX12_DESCRIPTOR_RANGE1 samplerRange;
                samplerRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

                CD3DX12_DESCRIPTOR_RANGE1 srvRange;
                srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

                // rootSignature
                CD3DX12_ROOT_PARAMETER1 rootParameters[3] = {};
                // constant buffer
                rootParameters[0].InitAsConstantBufferView(
                    1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
                    D3D12_SHADER_VISIBILITY_ALL);
                // sampler
                rootParameters[1].InitAsDescriptorTable(
                    1, &samplerRange, D3D12_SHADER_VISIBILITY_ALL);
                // srv
                rootParameters[2].InitAsDescriptorTable(
                    1, &srvRange, D3D12_SHADER_VISIBILITY_ALL);

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
                    IID_PPV_ARGS(&toneMappingPSO->root_signature));

                // shader
                ComPtr<ID3DBlob> tone_mappingVS;
                dx12::Util::CreateVertexShader(dx12::GpuCore::Instance().device,
                                               L"graphics/CombineVS.hlsl",
                                               tone_mappingVS);
                ComPtr<ID3DBlob> tone_mappingPS;
                dx12::Util::CreatePixelShader(dx12::GpuCore::Instance().device,
                                              L"graphics/CombinePS.hlsl",
                                              tone_mappingPS);
                // pipeline state
                D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
                psoDesc.InputLayout = {dx12::layout::combineIEs,
                                       _countof(dx12::layout::combineIEs)};
                psoDesc.pRootSignature = toneMappingPSO->root_signature;
                psoDesc.VS = CD3DX12_SHADER_BYTECODE(tone_mappingVS.Get());
                psoDesc.PS = CD3DX12_SHADER_BYTECODE(tone_mappingPS.Get());
                psoDesc.RasterizerState =
                    CD3DX12_RASTERIZER_DESC(dx12::rasterizer::postRS);
                psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
                psoDesc.DepthStencilState.StencilEnable = false;
                psoDesc.SampleMask = UINT_MAX;
                psoDesc.PrimitiveTopologyType =
                    D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
                psoDesc.NumRenderTargets = 1;
                psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
                psoDesc.SampleDesc.Count = 1;
                psoDesc.SampleDesc.Quality = 0;

                dx12::ThrowIfFailed(
                    dx12::GpuCore::Instance()
                        .device->CreateGraphicsPipelineState(
                            &psoDesc,
                            IID_PPV_ARGS(&toneMappingPSO->pipeline_state)));

                D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
                samplerHeapDesc.NumDescriptors = 1;
                samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
                samplerHeapDesc.Flags =
                    D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

                dx12::ThrowIfFailed(
                    dx12::GpuCore::Instance().device->CreateDescriptorHeap(
                        &samplerHeapDesc, IID_PPV_ARGS(&sampler_heap)));

                CD3DX12_CPU_DESCRIPTOR_HANDLE handle(
                    sampler_heap->GetCPUDescriptorHandleForHeapStart());
                UINT incrementSize =
                    dx12::GpuCore::Instance()
                        .device->GetDescriptorHandleIncrementSize(
                            D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

                dx12::GpuCore::Instance().device->CreateSampler(
                    &dx12::sampler::linearClampSS, handle);

                const_data.dx = 1.0f / foundation::Env::Instance().screen_width;
                const_data.dy =
                    1.0f / foundation::Env::Instance().screen_height;
                const_data.strength = 0.5f; // Bloom strength
                const_data.option1 = 1.0f;  // Exposure
                const_data.option2 = 2.2f;  // Gamma

                dx12::Util::CreateConstBuffer(const_data, const_buffer);
            }
            break;
        }
        case EnumStageType::eUpdate: {

            break;
        }
        case EnumStageType::eRender: {
            // Transition the input resource to a shader resource state
            auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                dx12::GpuCore::Instance().resource_LDR.Get(),
                D3D12_RESOURCE_STATE_RESOLVE_DEST,
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            command_list->ResourceBarrier(1, &barrier);

            auto handle_resolved =
                dx12::GpuCore::Instance()
                    .heap_LDR->GetGPUDescriptorHandleForHeapStart();
            auto handle_back_buffer = dx12::GpuCore::Instance().GetHandleFLIP();

            ID3D12DescriptorHeap *descriptorHeaps[] = {
                dx12::GpuCore::Instance().heap_LDR.Get(), sampler_heap.Get()};

            // Set the compute root signature and pipeline state
            command_list->RSSetViewports(1,
                                         &dx12::GpuCore::Instance().viewport);
            command_list->RSSetScissorRects(
                1, &dx12::GpuCore::Instance().scissorRect);
            command_list->OMSetRenderTargets(1, &handle_back_buffer, false,
                                             nullptr);
            command_list->SetGraphicsRootSignature(
                toneMappingPSO->root_signature);
            command_list->SetPipelineState(toneMappingPSO->pipeline_state);

            command_list->SetDescriptorHeaps(_countof(descriptorHeaps),
                                             descriptorHeaps);
            // 1:constant buffer
            command_list->SetGraphicsRootConstantBufferView(
                0, const_buffer->GetGPUVirtualAddress());
            // 2:sampler
            command_list->SetGraphicsRootDescriptorTable(
                1, sampler_heap->GetGPUDescriptorHandleForHeapStart());
            // 3:srv
            command_list->SetGraphicsRootDescriptorTable(2, handle_resolved);

            command_list->IASetPrimitiveTopology(
                D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            command_list->IASetVertexBuffers(0, 1, &mesh->vertex_buffer_view);
            command_list->IASetIndexBuffer(&mesh->index_buffer_view);
            command_list->DrawIndexedInstanced(mesh->index_count, 1, 0, 0, 0);
            // Transition the resources back to their original states
            barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                dx12::GpuCore::Instance().resource_LDR.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_COMMON);
            command_list->ResourceBarrier(1, &barrier);

            // barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            //     dx12::GpuCore::Instance().resource_LDR.Get(),
            //     D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            //     D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            // command_list->ResourceBarrier(1, &barrier);

            break;
        }
        default:
            break;
        }

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }

    __declspec(align(256)) struct ImageFilterConstData {
        float dx;
        float dy;
        float threshold;
        float strength;
        float option1;
        float option2;
        float option3;
        float option4;
    };
    std::shared_ptr<dx12::GraphicsPSO> toneMappingPSO;
    std::shared_ptr<Mesh> mesh;
    ComPtr<ID3D12DescriptorHeap> sampler_heap;
    ImageFilterConstData const_data;
    ComPtr<ID3D12Resource> const_buffer;
};
} // namespace core

#endif
