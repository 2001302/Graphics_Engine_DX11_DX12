#include "panel.h"
#include "pipeline_manager.h"
#include "game_object_node.h"

using namespace Engine;

EnumBehaviorTreeStatus GameObjectDetailNode::OnInvoke() {
    IDataBlock *block = DataBlock[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<Engine::PipelineManager *>(block);
    assert(manager != nullptr);

    return EnumBehaviorTreeStatus::eSuccess;
}
EnumBehaviorTreeStatus GameObjectDetailNode::OnShow() {
    IDataBlock *block = DataBlock[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<Engine::PipelineManager *>(block);
    assert(manager != nullptr);

    return EnumBehaviorTreeStatus::eSuccess;
}
