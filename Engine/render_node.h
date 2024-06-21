#ifndef _RENDERNODE
#define _RENDERNODE

#include "behavior_tree.h"

namespace Engine {
class InitializeCamera : public ActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class UpdateCamera : public ActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class InitializeImageBasedShader : public ActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class UpdateGameObjectsUsingImageBasedShader : public ActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class RenderGameObjectsUsingImageBasedShader : public ActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class InitializePhongShader : public ActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class UpdateGameObjectsUsingPhongShader : public ActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class RenderGameObjectsUsingPhongShader : public ActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class InitializeCubeMapShader : public ActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class UpdateCubeMap : public ActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class RenderCubeMap : public ActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};
} // namespace Engine
#endif