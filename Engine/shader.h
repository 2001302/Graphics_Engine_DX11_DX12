#ifndef _IShader
#define _IShader

#include "direct3D.h"
#include "light.h"
#include "material.h"
#include <map>

#define MAX_LIGHTS 3

namespace dx11 {

enum EnumShaderType {
    ePhong = 0,
    eCube = 1,
    eImageBased = 2,
    ePhysicallyBased = 3,
    eNormalGeometry = 4,
    eGround = 5,
};

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

} // namespace dx11
#endif
