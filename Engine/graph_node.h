#ifndef GRAPHNODE
#define GRAPHNODE

#include "common_struct.h"
#include "behavior_tree.h"

namespace Engine {

class GraphNode : public ActionNode {
  public:
    virtual EnumBehaviorTreeStatus Render(){};
};

} // namespace Engine
#endif