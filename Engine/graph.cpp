#include "graph.h"

using namespace Engine;

void Graph::Invoke() {}

void Graph::Show() { detail_node_->OnShow(); }

void Graph::SetDetailNode(std::shared_ptr<GraphNode> node) {
    detail_node_ = node;
}

std::shared_ptr<GraphNode> Graph::GetDetailNode() const { return detail_node_; }
