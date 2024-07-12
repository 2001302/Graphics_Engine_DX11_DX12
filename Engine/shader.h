#ifndef _IShader
#define _IShader

#include "common_struct.h"
#include "direct3D.h"

#define MAX_LIGHTS 3

namespace Engine {
struct IShaderSource {
  public:
    void Initialize() { InitializeThis(); }
    virtual void InitializeThis() {}
};
struct IShader {
    ComPtr<ID3D11VertexShader> vertex_shader;
    ComPtr<ID3D11PixelShader> pixel_shader;
    ComPtr<ID3D11InputLayout> layout;
    ComPtr<ID3D11SamplerState> sample_state;
    std::unordered_map<int, std::shared_ptr<IShaderSource>> source;
};

} // namespace Engine
#endif
