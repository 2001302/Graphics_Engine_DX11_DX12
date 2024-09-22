#ifndef _CAMERANODE
#define _CAMERANODE

#include "behavior_tree_builder.h"
#include "black_board.h"

namespace core {

class CameraNodeInvoker : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->job_context.get();

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

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace core

#endif