#ifndef _SHADOW_EFFECT_NODE
#define _SHADOW_EFFECT_NODE

#include "black_board.h"
#include "mesh_pso.h"
#include "mesh_renderer.h"
#include "skybox_pso.h"
#include "stencil_mark_pso.h"
#include <behavior_tree_builder.h>

namespace graphics {

class ShadowEffectNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto condition = black_board->conditions.get();
        auto targets = black_board->targets.get();

        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {

            break;
        }
        case EnumStageType::eUpdate: {

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

    ConstantBuffer<GlobalConstants> shadow_global_consts;
    std::shared_ptr<StencilMarkPSO> shadow_map_PSO;
};
} // namespace graphics

#endif
