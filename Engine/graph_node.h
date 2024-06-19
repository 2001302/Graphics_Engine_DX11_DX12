#ifndef GRAPHNODE
#define GRAPHNODE

#include "common_struct.h"
#include "behavior_tree.h"

namespace Engine {

class GraphNode : public ActionNode {
  public:
    virtual EnumBehaviorTreeStatus OnShow() {
        return EnumBehaviorTreeStatus::eSuccess;
    };
};

} // namespace Engine
#endif