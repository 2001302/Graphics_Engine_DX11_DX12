#ifndef GRAPH
#define GRAPH

#include "behavior_tree.h"
#include "common_struct.h"

namespace Engine {

class GraphNode {
  public:
    virtual EnumBehaviorTreeStatus OnInvoke() {
        return EnumBehaviorTreeStatus::eSuccess;
    };
    virtual EnumBehaviorTreeStatus OnShow() {
        return EnumBehaviorTreeStatus::eSuccess;
    };
};

class Graph {
  public:
    void Invoke();
    void Show();
    void SetDetailNode(std::shared_ptr<GraphNode> node);
    std::shared_ptr<GraphNode> GetDetailNode() const;
  private:
    std::shared_ptr<GraphNode> detail_node_;
    std::vector<std::shared_ptr<GraphNode>> behavior_node_;
};
} // namespace Engine
#endif