#ifndef BEHAVIORTREEBUILDER
#define BEHAVIORTREEBUILDER

#include "action_node.h"
#include "conditional_node.h"
#include "selector_node.h"
#include "sequence_node.h"
#include "animation_node.h"

namespace foundation {

class BehaviorTreeBuilder {
  public:
    BehaviorTreeBuilder *Build(foundation::IDataBlock *dataBlock) {
        tree = std::make_shared<SequenceNode>(dataBlock);
        current = tree.get();
        return this;
    }
    void Run() {
        tree->Invoke();
    }
    void Reset() { tree->Reset(); }
    BehaviorTreeBuilder *Excute(std::shared_ptr<BehaviorActionNode> node);
    BehaviorTreeBuilder *Sequence();
    BehaviorTreeBuilder *Selector();
    BehaviorTreeBuilder *Conditional(std::shared_ptr<ConditionalNode> node);
    BehaviorTreeBuilder *Close();

  private:
    std::shared_ptr<BehaviorActionNode> tree;
    BehaviorActionNode *current;
};
} // namespace common
#endif
