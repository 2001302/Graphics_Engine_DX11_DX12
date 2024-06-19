#ifndef _RENDERNODE
#define _RENDERNODE

#include "behavior_tree.h"

namespace Engine {
class InitializeCamera : public ActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class InitializePhongShader : public ActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
    HWND window;

  public:
    InitializePhongShader() { window = 0; };
    InitializePhongShader(HWND hwnd) { window = hwnd; };
};

class UpdateCamera : public ActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class UpdateGameObjects : public ActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class RenderGameObjects : public ActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};
} // namespace Engine
#endif