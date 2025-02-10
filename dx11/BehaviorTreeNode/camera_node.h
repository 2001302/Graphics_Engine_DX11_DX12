#ifndef _CAMERANODE
#define _CAMERANODE

#include "../pipeline/black_board.h"
#include <behaviortree/behavior_tree_builder.h>

namespace graphics {

class CameraNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->targets.get();

        switch (manager->stage_type) {
        case EnumStageType::eInitialize: {
            manager->camera = std::make_unique<Camera>();
            manager->camera->Initialize();
            manager->camera->Update();
            break;
        }
        case EnumStageType::eUpdate: {
            manager->camera->Update();
            break;
        }
        case EnumStageType::eRender: {

            graphics::Util::SetPipelineState(
                manager->draw_wire ? graphics::pipeline::defaultWirePSO
                                   : graphics::pipeline::defaultSolidPSO);
            graphics::Util::SetGlobalConsts(manager->global_consts_GPU);

            manager->camera->Draw();
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