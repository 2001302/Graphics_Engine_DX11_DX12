#ifndef _IMAGEFLITERSHADER
#define _IMAGEFLITERSHADER

#include "shader.h"
#include "behavior_tree_builder.h"

using namespace DirectX;

namespace Engine {
struct ImageFilterShader : public IShader {
  public:
};
struct ImageFilterShaderSource : public IShaderSource {
  public:

};

class InitializeBoardMap : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class RenderBoardMap : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

} // namespace Engine
#endif
