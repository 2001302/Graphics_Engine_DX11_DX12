#ifndef _NORMALGEOMETRYSHADER
#define _NORMALGEOMETRYSHADER

#include "shader.h"
#include "behavior_tree_builder.h"

using namespace DirectX;

namespace Engine {

struct NormalGeometryVertexConstantBuffer {

};

struct NormalGeometryPixelConstantBuffer {

};

class NormalGeometryShader : public IShader {
  public:
    ComPtr<ID3D11GeometryShader> normalGeometryShader;
};

class NormalGeometryShaderSource : public IShaderSource {
  public:
    ComPtr<ID3D11Buffer> vertex_constant_buffer;
    ComPtr<ID3D11Buffer> pixel_constant_buffer;
    NormalGeometryVertexConstantBuffer vertex_constant_buffer_data;
    NormalGeometryPixelConstantBuffer pixel_constant_buffer_data;

  private:
    void InitializeThis() override {
        Direct3D::GetInstance().CreateConstantBuffer(
            vertex_constant_buffer_data,
                                      vertex_constant_buffer);
        Direct3D::GetInstance().CreateConstantBuffer(pixel_constant_buffer_data,
                                      pixel_constant_buffer);
    }
};

class InitializeNormalGeometryShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};
} // namespace Engine
#endif