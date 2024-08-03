#ifndef _GUINODE
#define _GUINODE

#include "behavior_tree_builder.h"
#include "black_board.h"

namespace engine {
class InitializeImguiNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto gui = black_board->gui;
        gui->Initialize();

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class DrawSettingUiNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block.get();
        auto gui = black_board->gui;

        gui->PushNode(dynamic_cast<common::INode *>(manager));
        gui->Frame(manager);
        gui->ClearNode();

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace engine

#endif