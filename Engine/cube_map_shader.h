#ifndef _CUBEMAPSHADER
#define _CUBEMAPSHADER

#include <directxtk/DDSTextureLoader.h> 
#include "shader.h"

using namespace DirectX;

namespace Engine {
struct BasicVertexConstantBuffer {
    Matrix model;
    Matrix invTranspose;
    Matrix view;
    Matrix projection;
};

struct BasicPixelConstantBuffer {
    float dummy[4];
};

class CubeMapShader : public IShader {
  public:
    void CreateCubemapTexture(const wchar_t *filename,
                              ComPtr<ID3D11ShaderResourceView> &texResView);
};
class CubeMapShaderSource : public IShaderSource {
  public:
    ComPtr<ID3D11Buffer> vertex_constant_buffer;
    ComPtr<ID3D11Buffer> pixel_constant_buffer;
    BasicVertexConstantBuffer vertex_constant_buffer_data;
    BasicPixelConstantBuffer pixel_constant_buffer_data;

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
