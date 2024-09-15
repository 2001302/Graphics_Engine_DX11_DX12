#ifndef _SKYBOX_NODE
#define _SKYBOX_NODE

#include "black_board.h"
#include "common/behavior_tree_builder.h"
#include "mesh_renderer.h"
#include "skybox_pso.h"

namespace graphics {

class SkyBoxNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto condition = black_board->conditions.get();
        auto targets = black_board->targets.get();

        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {

            skyboxPSO = std::make_shared<SkyboxPSO>();
            skyboxPSO->Initialize();

            auto mesh_data = GeometryGenerator::MakeBox(40.0f);
            std::reverse(mesh_data.indices.begin(), mesh_data.indices.end());

            auto renderer = std::make_shared<MeshRenderer>();
            renderer->Initialize(std::vector{mesh_data}, false);

            skybox = std::make_shared<Model>();
            skybox->AddComponent(EnumComponentType::eRenderer, renderer);

            break;
        }
        case EnumStageType::eRender: {

            auto renderer = (MeshRenderer *)skybox->GetComponent(
                EnumComponentType::eRenderer);
            auto mesh = renderer->meshes.front();

            // skyboxPSO->Render(
            //     command_list, GpuDevice::Get().GetHandleHDR(),
            //     GpuDevice::Get().GetHandleDSV(),
            //     condition->shared_texture.get(), condition->gpu_heap.get(),
            //     condition->sampler_heap.get(),
            //     condition->global_consts.Get(), renderer->mesh_consts.Get(),
            //     renderer->material_consts.Get(), mesh->vertex_buffer_view,
            //     mesh->index_buffer_view, mesh->index_count);
            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }

    std::shared_ptr<SkyboxPSO> skyboxPSO;
    std::shared_ptr<Model> skybox;
};
} // namespace graphics

#endif
