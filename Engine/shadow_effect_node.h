#ifndef _SHADOW_EFFECT
#define _SHADOW_EFFECT

#include "behavior_tree_builder.h"
#include "black_board.h"

namespace engine {

class ShadowEffectNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block.get();

        switch (manager->stage_type) {
        case EnumStageType::eInitialize: {

            break;
        }
        case EnumStageType::eRender: {

            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace engine

#endif