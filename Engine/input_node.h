#ifndef _INPUTNODE
#define _INPUTNODE

#include "behavior_tree_builder.h"
#include "input.h"

namespace engine {
class InitializeInputNode : public BehaviorActionNode {
  public:
    InitializeInputNode(HINSTANCE hinstance) { this->hinstance = hinstance; };
    EnumBehaviorTreeStatus OnInvoke() override {

        auto input =
            dynamic_cast<Input *>(data_block[EnumDataBlockType::eInput]);
        assert(input != nullptr);

        input->Initialize(hinstance);

        return EnumBehaviorTreeStatus::eSuccess;
    }

  private:
    HINSTANCE hinstance;
};

class ReadInputNode : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto input =
            dynamic_cast<Input *>(data_block[EnumDataBlockType::eInput]);
        assert(input != nullptr);

        input->Frame();

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace engine

#endif