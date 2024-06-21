#ifndef _IMAGEBASEDSHADER
#define _IMAGEBASEDSHADER

#include "shader.h"

using namespace DirectX;

namespace Engine {

struct ImageBasedVertexConstantBuffer {
    Matrix model;
    Matrix invTranspose;
    Matrix view;
    Matrix projection;
};

struct ImageBasedPixelConstantBuffer {
    Vector3 eyeWorld;         // 12
    bool useTexture;          // 4
    Material material;        // 48
};

class ImageBasedShader : public IShader {
  public:
};

class ImageBasedShaderSource {
  public:
    ComPtr<ID3D11Buffer> vertex_constant_buffer;
    ComPtr<ID3D11Buffer> pixel_constant_buffer;
    ImageBasedVertexConstantBuffer vertex_constant_buffer_data;
    ImageBasedPixelConstantBuffer pixel_constant_buffer_data;
};
} // namespace Engine
#endif