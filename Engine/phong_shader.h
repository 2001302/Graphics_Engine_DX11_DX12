#ifndef _PHONGSHADER
#define _PHONGSHADER

#include "shader.h"

using namespace DirectX;

namespace Engine {
struct VertexConstantBuffer {
    Matrix model;
    Matrix invTranspose;
    Matrix view;
    Matrix projection;
};

#define MAX_LIGHTS 3

struct PixelConstantBuffer {
    Vector3 eyeWorld;         // 12
    bool useTexture;          // 4
    Material material;        // 48
    Light lights[MAX_LIGHTS]; // 48 * MAX_LIGHTS
    bool useBlinnPhong = true;
    Vector3 dummy;
};

/// <summary>
/// 공유되어 사용되는 Phong Shader
/// </summary>
class PhongShader : public IShader {
  public:
};

/// <summary>
/// Phong Shader에서 사용하는 공유되지 않는 Constant Buffer 정보
/// </summary>
class PhongShaderSource : public IShaderSource {
  public:
    ComPtr<ID3D11Buffer> vertex_constant_buffer;
    ComPtr<ID3D11Buffer> pixel_constant_buffer;
    VertexConstantBuffer vertex_constant_buffer_data;
    PixelConstantBuffer pixel_constant_buffer_data;

  private:
    void InitializeThis() override {
        // create constant buffer(Phong Shader)
        vertex_constant_buffer_data.model = Matrix();
        vertex_constant_buffer_data.view = Matrix();
        vertex_constant_buffer_data.projection = Matrix();

        shader_->CreateConstantBuffer(vertex_constant_buffer_data,
                                      vertex_constant_buffer);
        shader_->CreateConstantBuffer(pixel_constant_buffer_data,
                                      pixel_constant_buffer);
    }
};
} // namespace Engine
#endif