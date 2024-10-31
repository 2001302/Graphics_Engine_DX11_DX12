#ifndef _RENDERER_DRAW_NODE
#define _RENDERER_DRAW_NODE

#include "animator.h"
#include "black_board.h"
#include "mesh_pso.h"
#include "mesh_renderer.h"
#include "skybox_renderer.h"
#include <behavior_tree_builder.h>

namespace graphics {

class MeshObjectNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto condition = black_board->conditions.get();
        auto targets = black_board->targets.get();

        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {

            mesh_solid_PSO = std::make_shared<SolidMeshPSO>();
            mesh_solid_PSO->Initialize();
            skinned_mesh_solid_PSO = std::make_shared<SkinnedSolidMeshPSO>();
            skinned_mesh_solid_PSO->Initialize();
            break;
        }
        case EnumStageType::eUpdate: {

            auto context =
                GpuCore::Instance().GetCommand()->Begin<GraphicsCommandContext>(
                    L"MeshObjectNode:Update");

            for (auto &i : targets->objects) {

                Animator *animator = nullptr;
                if (i.second->TryGet(animator)) {
                    animator->Run(condition->delta_time);
                }

                MeshRenderer *renderer = nullptr;
                if (i.second->TryGet(renderer)) {
                    renderer->UpdateConstantBuffers();
                }
            }
            GpuCore::Instance().GetCommand()->Finish(context, true);

            break;
        }
        case EnumStageType::eRender: {
            for (auto &i : targets->objects) {
                MeshRenderer *renderer = nullptr;

                if (i.second->TryGet(renderer)) {

                    Animator *animator = nullptr;
                    if (i.second->TryGet(animator)) {
                        skinned_mesh_solid_PSO->Render(
                            targets->world.get(), condition->shared_sampler,
                            condition->global_consts.Get(), renderer, animator);
                    } else {
                        mesh_solid_PSO->Render(
                            targets->world.get(), condition->shared_sampler,
                            condition->global_consts.Get(), renderer);
                    }
                }
            }

            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }

    std::shared_ptr<SolidMeshPSO> mesh_solid_PSO;
    std::shared_ptr<WireMeshPSO> mesh_wire_PSO;
    std::shared_ptr<SkinnedSolidMeshPSO> skinned_mesh_solid_PSO;
    // std::shared_ptr<SkinnedWireMeshPSO> skinned_wire_solid_PSO;
};
} // namespace graphics

#endif
