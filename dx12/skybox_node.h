#ifndef _SKYBOX_NODE
#define _SKYBOX_NODE

#include "black_board.h"
#include "skybox_pso.h"
#include "skybox_renderer.h"
#include <behavior_tree_builder.h>

namespace graphics {

class SkyBoxNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto condition = black_board->conditions.get();
        auto targets = black_board->targets.get();

        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {

            skyboxPSO = std::make_shared<SolidSkyboxPSO>();
            skyboxPSO->Initialize();

            break;
        }
        case EnumStageType::eRender: {
            SkyboxRenderer *component = nullptr;
            if (targets->world->TryGet(component)) {
                skyboxPSO->Render(condition->shared_sampler,
                                  targets->world.get(),
                                  condition->global_consts.Get(), component);
            }
            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }

    std::shared_ptr<SolidSkyboxPSO> skyboxPSO;
};
} // namespace graphics

#endif
