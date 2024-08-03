#ifndef _CAMERANODE
#define _CAMERANODE

#include "behavior_tree_builder.h"
#include "black_board.h"

namespace engine {

class CameraNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block.get();

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

            GraphicsManager::Instance().SetPipelineState(
                manager->draw_wire ? Graphics::defaultWirePSO
                                   : Graphics::defaultSolidPSO);
            GraphicsManager::Instance().SetGlobalConsts(
                manager->global_consts_GPU);

            manager->camera->Draw();
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