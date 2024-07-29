#ifndef BEHAVIORTREEBUILDER
#define BEHAVIORTREEBUILDER

#include "action_node.h"
#include "conditional_node.h"
#include "parallel_node.h"
#include "selector_node.h"
#include "sequence_node.h"

namespace engine {

class BehaviorTreeBuilder {
  public:
    BehaviorTreeBuilder *
    Build(std::map<EnumDataBlockType, common::IDataBlock *> dataBlock) {
        tree = std::make_shared<SequenceNode>(dataBlock);
        current = tree.get();
        return this;
    }
    void Run() {
        tree->Invoke();
        tree->Dispose();
        tree.reset();
    }
    BehaviorTreeBuilder *Excute(std::shared_ptr<BehaviorActionNode> node);
    BehaviorTreeBuilder *Sequence();
    BehaviorTreeBuilder *Selector();
    BehaviorTreeBuilder *Close();
    BehaviorTreeBuilder *Conditional(std::shared_ptr<ConditionalNode> node);
    BehaviorTreeBuilder *End();
    BehaviorTreeBuilder *Parallel(std::map<int, common::INode *> target_ids);

  private:
    std::shared_ptr<BehaviorActionNode> tree;
    BehaviorActionNode *current;
    EnumConditionalStatus conditional_status = EnumConditionalStatus::ePass;
};
} // namespace engine
#endif
