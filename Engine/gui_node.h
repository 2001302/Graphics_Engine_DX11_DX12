#ifndef _GUINODE
#define _GUINODE

#include "behavior_tree_builder.h"
#include "black_board.h"

namespace engine {

class GuiNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block.get();
        auto gui = black_board->gui;

        switch (manager->stage_type) {
        case EnumStageType::eInitialize: {
            gui->Initialize();
            break;
        }
        case EnumStageType::eRender: {
            gui->PushNode(dynamic_cast<common::INode *>(manager));
            gui->Frame(manager);
            gui->ClearNode();

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