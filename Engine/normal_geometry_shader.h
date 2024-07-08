#ifndef _NORMALGEOMETRYSHADER
#define _NORMALGEOMETRYSHADER

#include "shader.h"
#include "behavior_tree_builder.h"

using namespace DirectX;

namespace Engine {

struct NormalGeometryShader : public IShader {
  public:
    struct NormalGeometryVertexConstantBuffer {};

    struct NormalGeometryPixelConstantBuffer {};

    ComPtr<ID3D11GeometryShader> normalGeometryShader;
};

struct NormalGeometryShaderSource : public IShaderSource {
  public:
    ComPtr<ID3D11Buffer> vertex_constant_buffer;
    ComPtr<ID3D11Buffer> pixel_constant_buffer;
    NormalGeometryShader::NormalGeometryVertexConstantBuffer
        vertex_constant_buffer_data;
    NormalGeometryShader::NormalGeometryPixelConstantBuffer
        pixel_constant_buffer_data;

  private:
    void InitializeThis() override;
};

class InitializeNormalGeometryShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};
} // namespace Engine
#endif