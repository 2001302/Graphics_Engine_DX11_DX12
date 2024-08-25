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
                toneMappingPSO = std::make_shared<dx12::ComputePSO>();

                // shader
                ComPtr<ID3DBlob> tone_mappingCS;
                dx12::Util::CreateComputeShader(
                    dx12::GpuCore::Instance().device,
                    L"graphics/ToneMappingCS.hlsl", tone_mappingCS);

                // t0
                CD3DX12_DESCRIPTOR_RANGE1 srvRange;
                srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
                // u0
                CD3DX12_DESCRIPTOR_RANGE1 uavRange;
                uavRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

                // rootSignature
                CD3DX12_ROOT_PARAMETER1 rootParameters[3] = {};
                // 1:constant buffer
                rootParameters[0].InitAsConstantBufferView(
                    0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
                    D3D12_SHADER_VISIBILITY_ALL);
                // 2:resolved buffer
                rootParameters[1].InitAsDescriptorTable(
                    1, &srvRange, D3D12_SHADER_VISIBILITY_ALL);
                // 3:resolved buffer
                rootParameters[2].InitAsDescriptorTable(
                    1, &uavRange, D3D12_SHADER_VISIBILITY_ALL);

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

                // pipeline state
                D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
                psoDesc.pRootSignature = toneMappingPSO->root_signature;
                psoDesc.CS = CD3DX12_SHADER_BYTECODE(tone_mappingCS.Get());

                dx12::ThrowIfFailed(
                    dx12::GpuCore::Instance()
                        .device->CreateComputePipelineState(
                            &psoDesc,
                            IID_PPV_ARGS(&toneMappingPSO->pipeline_state)));
            }
            break;
        }
        case EnumStageType::eUpdate: {

            break;
        }
        case EnumStageType::eRender: {
            // Transition the input resource to a shader resource state
            auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                dx12::GpuCore::Instance().resource_resolved.Get(),
                D3D12_RESOURCE_STATE_RESOLVE_DEST,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            command_list->ResourceBarrier(1, &barrier);

            // Set the compute root signature and pipeline state
            command_list->SetComputeRootSignature(
                toneMappingPSO->root_signature);
            command_list->SetPipelineState(toneMappingPSO->pipeline_state);

            auto handle_resolved =
                dx12::GpuCore::Instance()
                    .heap_resolved->GetGPUDescriptorHandleForHeapStart();
            auto handle_staging_buffer =
                dx12::GpuCore::Instance()
                    .heap_staging->GetGPUDescriptorHandleForHeapStart();

            ID3D12DescriptorHeap *descriptorHeaps[] = {
                dx12::GpuCore::Instance().heap_resolved.Get(),
                dx12::GpuCore::Instance().heap_staging.Get()};

            command_list->SetDescriptorHeaps(_countof(descriptorHeaps),
                                             descriptorHeaps);
            const float exposure = 1.0f;
            // 1:constants
            command_list->SetComputeRoot32BitConstants(0, 1, &exposure, 0);
            // 2:resolved buffer
            command_list->SetComputeRootDescriptorTable(1, handle_resolved);
            // 3:staging buffer
            command_list->SetComputeRootDescriptorTable(2,
                                                        handle_staging_buffer);

            // Dispatch the compute shader
            D3D12_RESOURCE_DESC inputDesc =
                dx12::GpuCore::Instance().resource_resolved->GetDesc();
            command_list->Dispatch((inputDesc.Width + 15) / 16,
                                   (inputDesc.Height + 15) / 16, 1);

            // Transition the resources back to their original states
            barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                dx12::GpuCore::Instance().resource_resolved.Get(),
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_RENDER_TARGET);
            command_list->ResourceBarrier(1, &barrier);

            break;
        }
        default:
            break;
        }

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }

    std::shared_ptr<dx12::ComputePSO> toneMappingPSO;
};
} // namespace core

#endif
