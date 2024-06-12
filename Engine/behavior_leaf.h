#ifndef _BehaviorLeaf
#define _BehaviorLeaf

#include "behavior_tree.h"

namespace Engine {
class InitializeCamera : public ActionNode {
    EnumBehaviorTreeStatus Invoke() override;
};

class InitializePhongShader : public ActionNode {
    EnumBehaviorTreeStatus Invoke() override;
    HWND window;

  public:
    InitializePhongShader() { window = 0; };
    InitializePhongShader(HWND hwnd) { window = hwnd; };
};

class RenderGameObjects : public ActionNode {
    EnumBehaviorTreeStatus Invoke() override;
};
} // namespace Engine
#endif