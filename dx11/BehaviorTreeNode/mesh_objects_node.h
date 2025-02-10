#ifndef _RENDERER_DRAW_NODE
#define _RENDERER_DRAW_NODE

#include "../pipeline/black_board.h"
#include "../component/mesh_renderer.h"
#include "../component/skinned_mesh_renderer.h"
#include "../component/animator.h"
#include <behaviortree/behavior_tree_builder.h>

namespace graphics {

class MeshObjectNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto targets = black_board->targets;
        auto gui = black_board->gui;

        switch (targets->stage_type) {
        case EnumStageType::eInitialize: {

            break;
        }
        case EnumStageType::eUpdate: {

            for (auto &i : targets->objects) {
                
                MeshRenderer *mesh_renderer = nullptr;
                SkinnedMeshRenderer *skinned_mesh_renderer = nullptr;
                if (i.second->TryGet(mesh_renderer)) {
                    mesh_renderer->UpdateConstantBuffers();
                }
                if (i.second->TryGet(skinned_mesh_renderer)) {
                    Animator *animator = nullptr;
                    if (i.second->TryGet(animator))
                    {
                        animator->Run(targets->delta_time);
                        skinned_mesh_renderer->UpdateConstantBuffers();
                    }
                }
            }

            break;
        }
        case EnumStageType::eRender: {

            graphics::Util::SetPipelineState(
                targets->draw_wire ? graphics::pipeline::defaultWirePSO
                                   : graphics::pipeline::defaultSolidPSO);
            graphics::Util::SetGlobalConsts(targets->global_consts_GPU);

            for (auto &i : targets->objects) {
                MeshRenderer *renderer = nullptr;
                if (i.second->TryGet(renderer)) {
                    renderer->Render();
                }
            }

            MirrorRenderer *mirror = nullptr;
            if (targets->ground->TryGet(mirror)) {
                if (mirror->GetMirrorAlpha() == 1.0f) {
                    mirror->Render();
                }
            }

            graphics::Util::SetPipelineState(
                targets->draw_wire ? graphics::pipeline::skinnedWirePSO
                                       : graphics::pipeline::skinnedSolidPSO);

            for (auto &i : targets->objects) {
                SkinnedMeshRenderer *renderer = nullptr;
                Animator *animator = nullptr;
                if (i.second->TryGet(renderer) && i.second->TryGet(animator)) {
                    animator->UploadBoneData();
                    renderer->Render();
                }
            }

            //todo: skinned mirror object

            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};
}; // namespace graphics

#endif