#ifndef _RENDERNODE
#define _RENDERNODE

#include "behavior_tree.h"
#include "conditional_node.h"

namespace Engine {
class InitializeCamera : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class UpdateCamera : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class CheckImageBasedShader : public ConditionalNode {
    EnumBehaviorTreeStatus CheckCondition() override;
};

class InitializeImageBasedShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class UpdateGameObjectsUsingImageBasedShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class RenderGameObjectsUsingImageBasedShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class CheckPhongShader : public ConditionalNode {
    EnumBehaviorTreeStatus CheckCondition() override;
};

class InitializePhongShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class UpdateGameObjectsUsingPhongShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class RenderGameObjectsUsingPhongShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class InitializeCubeMapShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class UpdateCubeMap : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class RenderCubeMap : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class CheckPhysicallyBasedShader : public ConditionalNode {
    EnumBehaviorTreeStatus CheckCondition() override;
};

class InitializePhysicallyBasedShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class UpdateGameObjectsUsingPhysicallyBasedShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class RenderGameObjectsUsingPhysicallyBasedShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class InitializeNormalGeometryShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

} // namespace Engine
#endif