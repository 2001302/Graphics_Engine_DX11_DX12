#ifndef _CAMERANODE
#define _CAMERANODE

#include "black_board.h"
#include <behavior_tree_builder.h>

namespace graphics {

class CameraNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto condition = black_board->conditions.get();
        auto targets = black_board->targets.get();

        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {
            targets->camera->Initialize();
            targets->camera->Update();
            break;
        }
        case EnumStageType::eUpdate: {
            targets->camera->Update();
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
