#ifndef _GAMEOBJECTNODE
#define _GAMEOBJECTNODE

#include "behavior_tree.h"

namespace Engine {
class InspectorNode : public ActionNode {
    EnumBehaviorTreeStatus Invoke() override;
};

} // namespace Engine
#endif