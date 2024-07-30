#ifndef _GUINODE
#define _GUINODE

#include "behavior_tree_builder.h"
#include "setting_ui.h"

namespace engine {
class InitializeImguiNode : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto gui = dynamic_cast<common::SettingUi *>(
            data_block[EnumDataBlockType::eGui]);
        assert(gui != nullptr);

        gui->Initialize();

        return EnumBehaviorTreeStatus::eSuccess;
    }
};
} // namespace engine

#endif