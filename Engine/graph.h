#ifndef GRAPH
#define GRAPH

#include "common_struct.h"
#include "graph_node.h"

namespace Engine {

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