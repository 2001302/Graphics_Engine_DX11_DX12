#ifndef GRAPH
#define GRAPH

#include "common_struct.h"
#include "graph_node.h"

namespace Engine {

class Graph {
  public:
    void Invoke(){};
    void Show(){};
  private:
    GraphNode detail_node_;
    std::vector<GraphNode> behavior_node_;
};
} // namespace Engine
#endif