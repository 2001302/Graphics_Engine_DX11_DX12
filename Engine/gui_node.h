#ifndef _GUINODE
#define _GUINODE

#include "behavior_tree_builder.h"
#include "setting_ui.h"

namespace engine {
class InitializeImguiNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto gui = dynamic_cast<common::SettingUi *>(
            data_block[common::EnumDataBlockType::eGui]);
        assert(gui != nullptr);

        gui->Initialize();

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class DrawSettingUiNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[common::EnumDataBlockType::eRenderBlock]);
        assert(manager != nullptr);

        auto gui = dynamic_cast<common::SettingUi *>(
            data_block[common::EnumDataBlockType::eGui]);
        assert(gui != nullptr);

        gui->PushNode(dynamic_cast<common::INode *>(manager));
        gui->Frame(manager);
        gui->ClearNode();

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace engine

#endif