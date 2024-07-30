#ifndef _INPUTNODE
#define _INPUTNODE

#include "behavior_tree_builder.h"
#include "input.h"

namespace engine {
class InitializeInputNode : public common::BehaviorActionNode {
  public:
    InitializeInputNode(HINSTANCE hinstance) { this->hinstance = hinstance; };
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto input = dynamic_cast<Input *>(
            data_block[common::EnumDataBlockType::eInput]);
        assert(input != nullptr);

        input->Initialize(hinstance);

        return common::EnumBehaviorTreeStatus::eSuccess;
    }

  private:
    HINSTANCE hinstance;
};

class ReadInputNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto input = dynamic_cast<Input *>(
            data_block[common::EnumDataBlockType::eInput]);
        assert(input != nullptr);

        input->Frame();

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace engine

#endif