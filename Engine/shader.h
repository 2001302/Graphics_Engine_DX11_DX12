#ifndef _IShader
#define _IShader

#include "common_struct.h"
#include "direct3D.h"

#define MAX_LIGHTS 3

namespace Engine {
struct IShader {
    ComPtr<ID3D11VertexShader> vertex_shader;
    ComPtr<ID3D11PixelShader> pixel_shader;
    ComPtr<ID3D11InputLayout> layout;
    ComPtr<ID3D11SamplerState> sample_state;
};

struct IShaderSource {
  public:
    void Initialize() {
        InitializeThis();
    }
    virtual void InitializeThis() {}
};
} // namespace Engine
#endif
