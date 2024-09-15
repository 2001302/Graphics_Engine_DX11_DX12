#ifndef _TONE_MAPPING_NODE
#define _TONE_MAPPING_NODE

#include "black_board.h"
#include "common/behavior_tree_builder.h"
#include "mesh_renderer.h"
#include "tone_mapping_pso.h"

namespace graphics {

class ToneMappingNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto condition = black_board->conditions.get();
        auto targets = black_board->targets.get();

        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {

            // toneMappingPSO = std::make_shared<ToneMappingPSO>();
            // toneMappingPSO->Initialize();

            // auto mesh_data = GeometryGenerator::MakeSquare();
            // mesh = std::make_shared<Mesh>();

            // Util::CreateVertexBuffer(mesh_data.vertices, mesh->vertex_buffer,
            //                          mesh->vertex_buffer_view);
            // mesh->index_count = UINT(mesh_data.indices.size());
            // mesh->vertex_count = UINT(mesh_data.vertices.size());
            // mesh->stride = UINT(sizeof(Vertex));
            // Util::CreateIndexBuffer(mesh_data.indices, mesh->index_buffer,
            //                         mesh->index_buffer_view);

            // const_data.strength = 0.5f;
            // const_data.exposure = 1.0f;
            // const_data.gamma = 2.2f;

            // Util::CreateConstBuffer(const_data, const_buffer);

            // D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
            // samplerHeapDesc.NumDescriptors = 1;
            // samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
            // samplerHeapDesc.Flags =
            // D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

            // ThrowIfFailed(GpuDevice::Get().device->CreateDescriptorHeap(
            //     &samplerHeapDesc, IID_PPV_ARGS(&sampler_heap)));

            // CD3DX12_CPU_DESCRIPTOR_HANDLE handle(
            //     sampler_heap->GetCPUDescriptorHandleForHeapStart());
            // UINT incrementSize =
            //     GpuDevice::Get().device->GetDescriptorHandleIncrementSize(
            //         D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

            // GpuDevice::Get().device->CreateSampler(&sampler::linearClampSS,
            //                                           handle);

            break;
        }
        case EnumStageType::eUpdate: {

            break;
        }
        case EnumStageType::eRender: {
            // toneMappingPSO->Render(command_list, sampler_heap, const_buffer,
            //                        mesh->vertex_buffer_view,
            //                        mesh->index_buffer_view,
            //                        mesh->index_count);
            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }

    __declspec(align(256)) struct ImageFilterConstData {
        float threshold;
        float strength;
        float exposure;
        float gamma;
    };
    std::shared_ptr<ToneMappingPSO> toneMappingPSO;
    std::shared_ptr<Mesh> mesh;
    ComPtr<ID3D12DescriptorHeap> sampler_heap;
    ImageFilterConstData const_data;
    ComPtr<ID3D12Resource> const_buffer;
};
} // namespace graphics

#endif
