#ifndef _GRAPHICSPSO
#define _GRAPHICSPSO

#include "graphics_util.h"

namespace engine {

class GraphicsPSO {
  public:
    void operator=(const GraphicsPSO &pso);
    void SetBlendFactor(const float blendFactor[4]);

  public:
    ComPtr<ID3D11VertexShader> vertex_shader;
    ComPtr<ID3D11PixelShader> pixel_shader;
    ComPtr<ID3D11HullShader> hull_shader;
    ComPtr<ID3D11DomainShader> domain_shader;
    ComPtr<ID3D11GeometryShader> geometry_shader;
    ComPtr<ID3D11InputLayout> input_layout;

    ComPtr<ID3D11BlendState> blend_state;
    ComPtr<ID3D11DepthStencilState> depth_stencil_state;
    ComPtr<ID3D11RasterizerState> rasterizer_state;

    float blend_factor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    UINT stencil_ref = 0;

    D3D11_PRIMITIVE_TOPOLOGY primitive_topology =
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

} // namespace engine
#endif