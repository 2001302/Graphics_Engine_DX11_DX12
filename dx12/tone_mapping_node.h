#ifndef _TONE_MAPPING_NODE
#define _TONE_MAPPING_NODE

#include "black_board.h"
#include "mesh_renderer.h"
#include "tone_mapping_pso.h"
#include <behavior_tree_builder.h>

namespace graphics {

class ToneMappingNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto condition = black_board->conditions.get();
        auto targets = black_board->targets.get();

        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {

            toneMappingPSO = std::make_shared<ToneMappingPSO>();
            toneMappingPSO->Initialize();

            auto mesh_data = GeometryGenerator::MakeSquare();
            mesh = std::make_shared<Mesh>();

            Util::CreateVertexBuffer(mesh_data.vertices, mesh->vertex_buffer,
                                     mesh->vertex_buffer_view);
            mesh->index_count = UINT(mesh_data.indices.size());
            mesh->vertex_count = UINT(mesh_data.vertices.size());
            mesh->stride = UINT(sizeof(Vertex));
            Util::CreateIndexBuffer(mesh_data.indices, mesh->index_buffer,
                                    mesh->index_buffer_view);

            const_buffer.Initialize();

            sampler = SamplerState::Create({sampler::linearClampSS});

            break;
        }
        case EnumStageType::eUpdate: {
            const_buffer.Upload();
            break;
        }
        case EnumStageType::eRender: {
            toneMappingPSO->Render(const_buffer.Get(), mesh.get(), sampler);
            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }

    __declspec(align(256)) struct ImageFilterConstData {
        float threshold = 0.0f;
        float strength = 0.5f;
        float exposure = 1.0f;
        float gamma = 2.2f;
    };
    std::shared_ptr<ToneMappingPSO> toneMappingPSO;
    std::shared_ptr<Mesh> mesh;
    SamplerState *sampler;
    ConstantBuffer<ImageFilterConstData> const_buffer;
};
} // namespace graphics

#endif
