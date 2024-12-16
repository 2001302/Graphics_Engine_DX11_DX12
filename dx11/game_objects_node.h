#ifndef _RENDERER_DRAW_NODE
#define _RENDERER_DRAW_NODE

#include "black_board.h"
#include "mesh_renderer.h"
#include "skinned_mesh_renderer.h"
#include <behavior_tree_builder.h>

namespace graphics {

class GameObjectNodeInvoker : public common::BehaviorActionNode {
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
                MeshRenderer *renderer = nullptr;
                if (i.second->TryGet(renderer)) {
                    renderer->UpdateConstantBuffers();
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

            graphics::Util::SetPipelineState(graphics::pipeline::normalsPSO);
            for (auto &i : targets->objects) {
                MeshRenderer *renderer = nullptr;
                if (i.second->TryGet(renderer)) {
                    if (renderer->draw_normals)
                        renderer->RenderNormals();
                }
            }

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