#ifndef _NORMALGEOMETRYSHADER
#define _NORMALGEOMETRYSHADER

#include "behavior_tree_builder.h"
#include "shader.h"

using namespace DirectX;

namespace dx11 {

struct NormalGeometryShader : public IShader {
  public:
    struct NormalGeometryVertexConstantBuffer {};

    struct NormalGeometryPixelConstantBuffer {};

    ComPtr<ID3D11GeometryShader> normalGeometryShader;
};

struct NormalGeometryShaderSource : public IConstantBufferData {
  public:
    ComPtr<ID3D11Buffer> vertex_constant_buffer;
    ComPtr<ID3D11Buffer> pixel_constant_buffer;
    NormalGeometryShader::NormalGeometryVertexConstantBuffer
        vertex_constant;
    NormalGeometryShader::NormalGeometryPixelConstantBuffer
        pixel_constant;

  private:
    void InitializeThis() override;
};

class InitializeNormalGeometryShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};
} // namespace dx11
#endif
