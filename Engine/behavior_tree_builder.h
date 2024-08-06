#ifndef BEHAVIORTREEBUILDER
#define BEHAVIORTREEBUILDER

#include "action_node.h"
#include "conditional_node.h"
#include "parallel_node.h"
#include "selector_node.h"
#include "sequence_node.h"

namespace common {

class BehaviorTreeBuilder {
  public:
    BehaviorTreeBuilder *Build(common::IDataBlock *dataBlock) {
        tree = std::make_shared<SequenceNode>(dataBlock);
        current = tree.get();
        return this;
    }
    void Run() {
        tree->Invoke();
        tree.reset();
    }
    BehaviorTreeBuilder *Excute(std::shared_ptr<BehaviorActionNode> node);
    BehaviorTreeBuilder *Sequence();
    BehaviorTreeBuilder *Selector();
    BehaviorTreeBuilder *Conditional(std::shared_ptr<ConditionalNode> node);
    BehaviorTreeBuilder *Loop(std::map<int, common::INode *> target_ids);
    BehaviorTreeBuilder *Close();

  private:
    std::shared_ptr<BehaviorActionNode> tree;
    BehaviorActionNode *current;
};
} // namespace common
#endif
