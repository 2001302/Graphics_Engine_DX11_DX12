#ifndef _SKYBOXNODE
#define _SKYBOXNODE

#include "black_board.h"
#include "mesh_renderer.h"
#include "skybox_renderer.h"
#include <behaviortree/behavior_tree_builder.h>

namespace graphics {

class SkyboxNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto targets = black_board->targets.get();

        switch (targets->stage_type) {
        case EnumStageType::eInitialize: {
            
            break;
        }
        case EnumStageType::eRender: {

            graphics::Util::SetPipelineState(
                targets->draw_wire ? graphics::pipeline::skyboxWirePSO
                                   : graphics::pipeline::skyboxSolidPSO);
            SkyboxRenderer *renderer = nullptr;
            if (targets->skybox->TryGet(renderer)) {
                renderer->Render();
            }
            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace graphics

#endif