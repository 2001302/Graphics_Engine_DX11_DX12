#ifndef _IMAGEBASEDSHADER
#define _IMAGEBASEDSHADER

#include "shader.h"
#include "behavior_tree_builder.h"

using namespace DirectX;

namespace Engine {
struct ImageBasedShader : public IShader {
  public:
    struct ImageBasedVertexConstantBuffer {
        Matrix model;
        Matrix invTranspose;
        Matrix view;
        Matrix projection;
    };

    struct ImageBasedPixelConstantBuffer {
        Vector3 eyeWorld;  // 12
        bool useTexture;   // 4
        Material material; // 48
    };
};

struct ImageBasedShaderSource : public IShaderSource {
  public:
    ComPtr<ID3D11Buffer> vertex_constant_buffer;
    ComPtr<ID3D11Buffer> pixel_constant_buffer;
    ImageBasedShader::ImageBasedVertexConstantBuffer vertex_constant_buffer_data;
    ImageBasedShader::ImageBasedPixelConstantBuffer pixel_constant_buffer_data;

  private:
    void InitializeThis() override;
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

} // namespace Engine
#endif
