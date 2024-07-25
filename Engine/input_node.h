#ifndef _INPUTNODE
#define _INPUTNODE

#include "behavior_tree_builder.h"
#include "input.h"

namespace engine {
class InitializeInput : public BehaviorActionNode {
  public:
    InitializeInput(HINSTANCE hinstance) { this->hinstance = hinstance; };
    EnumBehaviorTreeStatus OnInvoke() override {

        auto input = dynamic_cast<Input *>(
            data_block[EnumDataBlockType::eInput]);
        assert(input != nullptr);

        input->Initialize(hinstance);

        return EnumBehaviorTreeStatus::eSuccess;
    }

  private:
    HINSTANCE hinstance;
};
} // namespace engine

#endif