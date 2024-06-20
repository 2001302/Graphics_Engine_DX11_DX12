#ifndef _CUBEMAPSHADER
#define _CUBEMAPSHADER

#include <directxtk/DDSTextureLoader.h> 
#include "shader.h"

using namespace DirectX;

namespace Engine {
class CubeMapShader : public IShader {
  public:
    void CreateCubemapTexture(const wchar_t *filename,
                              ComPtr<ID3D11ShaderResourceView> &texResView);
};
class CubeMapShaderSource {
  public:
    ComPtr<ID3D11Buffer> vertex_constant_buffer;
    ComPtr<ID3D11Buffer> pixel_constant_buffer;
    BasicVertexConstantBuffer vertex_constant_buffer_data;
    BasicPixelConstantBuffer pixel_constant_buffer_data;
};
} // namespace Engine
#endif
