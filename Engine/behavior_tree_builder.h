#ifndef BEHAVIORTREEBUILDER
#define BEHAVIORTREEBUILDER

#include "common_struct.h"
#include "behavior_tree.h"
#include "sequence_node.h"
#include "selector_node.h"
#include "conditional_node.h"

namespace Engine {

class BehaviorTreeBuilder {
  public:
    BehaviorTreeBuilder*
    Build(std::map<EnumDataBlockType, IDataBlock *> dataBlock) {
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

  private:
    std::shared_ptr<BehaviorActionNode> tree;
    BehaviorActionNode* current;
    EnumConditionalStatus conditional_status;
};
} // namespace Engine
#endif
