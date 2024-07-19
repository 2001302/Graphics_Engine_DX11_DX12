#ifndef _IMAGEFLITERSHADER
#define _IMAGEFLITERSHADER

#include "behavior_tree_builder.h"
#include "shader.h"

using namespace DirectX;

namespace engine {
struct ImageFilterShader : public IShader {
  public:
};
struct ImageFilterShaderSource : public IConstantBufferData {
  public:
};

class InitializeBoardMap : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class RenderBoardMap : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

} // namespace dx11
#endif
