#ifndef _CUBEMAPSHADER
#define _CUBEMAPSHADER

#include <directxtk/DDSTextureLoader.h> 
#include "shader.h"

using namespace DirectX;

namespace Engine {
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

class CubeMapShader : public IShader {
  public:
    void CreateDDSTexture(const wchar_t *filename,
                              ComPtr<ID3D11ShaderResourceView> &texResView);
};
class CubeMapShaderSource : public IShaderSource {
  public:
    ComPtr<ID3D11Buffer> vertex_constant_buffer;
    ComPtr<ID3D11Buffer> pixel_constant_buffer;
    CubeMapVertexConstantBuffer vertex_constant_buffer_data;
    CubeMapPixelConstantBuffer pixel_constant_buffer_data;

  private:
    void InitializeThis() override {
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
