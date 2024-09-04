#ifndef _TONE_MAPPING_NODE
#define _TONE_MAPPING_NODE

#include "foundation/behavior_tree_builder.h"
#include "command_pool.h"
#include "tone_mapping_pso.h"
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

            toneMappingPSO = std::make_shared<dx12::ToneMappingPSO>();
            toneMappingPSO->Initialize();

            auto mesh_data = GeometryGenerator::MakeSquare();
            mesh = std::make_shared<dx12::Mesh>();

            dx12::Util::CreateVertexBuffer(mesh_data.vertices,
                                           mesh->vertex_buffer,
                                           mesh->vertex_buffer_view);
            mesh->index_count = UINT(mesh_data.indices.size());
            mesh->vertex_count = UINT(mesh_data.vertices.size());
            mesh->stride = UINT(sizeof(dx12::Vertex));
            dx12::Util::CreateIndexBuffer(mesh_data.indices, mesh->index_buffer,
                                          mesh->index_buffer_view);

            const_data.strength = 0.5f;
            const_data.exposure = 1.0f;
            const_data.gamma = 2.2f; 

            dx12::Util::CreateConstBuffer(const_data, const_buffer);

            D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
            samplerHeapDesc.NumDescriptors = 1;
            samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
            samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

            dx12::ThrowIfFailed(
                dx12::GpuCore::Instance().device->CreateDescriptorHeap(
                    &samplerHeapDesc, IID_PPV_ARGS(&sampler_heap)));

            CD3DX12_CPU_DESCRIPTOR_HANDLE handle(
                sampler_heap->GetCPUDescriptorHandleForHeapStart());
            UINT incrementSize = dx12::GpuCore::Instance()
                                     .device->GetDescriptorHandleIncrementSize(
                                         D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

            dx12::GpuCore::Instance().device->CreateSampler(
                &dx12::sampler::linearClampSS, handle);

            break;
        }
        case EnumStageType::eUpdate: {

            break;
        }
        case EnumStageType::eRender: {
            toneMappingPSO->Render(command_list, sampler_heap, const_buffer,
                                   mesh->vertex_buffer_view,
                                   mesh->index_buffer_view, mesh->index_count);
            break;
        }
        default:
            break;
        }

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }

    __declspec(align(256)) struct ImageFilterConstData {
        float threshold;
        float strength;
        float exposure;
        float gamma;
    };
    std::shared_ptr<dx12::ToneMappingPSO> toneMappingPSO;
    std::shared_ptr<dx12::Mesh> mesh;
    ComPtr<ID3D12DescriptorHeap> sampler_heap;
    ImageFilterConstData const_data;
    ComPtr<ID3D12Resource> const_buffer;
};
} // namespace core

#endif
