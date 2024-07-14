#ifndef _CUBEMAPSHADER
#define _CUBEMAPSHADER

#include "behavior_tree_builder.h"
#include "shader.h"

using namespace DirectX;

namespace dx11 {

struct CubeMapShader : public IShader {
  public:
    struct CubeMapVertexConstantBuffer {
        Matrix model;
        Matrix invTranspose;
        Matrix view;
        Matrix projection;
    };

    struct CubeMapPixelConstantBuffer {
        int textureToDraw = 0; // 0: Env, 1: Specular, 2: Irradiance
        float mipLevel = 0.0f;
        float dummy1;
        float dummy2;
    };
};
struct CubeMapShaderSource : public IShaderSource {
  public:
    ComPtr<ID3D11Buffer> vertex_constant_buffer;
    ComPtr<ID3D11Buffer> pixel_constant_buffer;
    CubeMapShader::CubeMapVertexConstantBuffer vertex_constant_buffer_data;
    CubeMapShader::CubeMapPixelConstantBuffer pixel_constant_buffer_data;

  private:
    void InitializeThis() override;
};

class InitializeCubeMapShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class CheckCubeMapShader : public ConditionalNode {
    EnumBehaviorTreeStatus CheckCondition() override;
};

class UpdateCubeMap : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class RenderCubeMap : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

} // namespace dx11
#endif
