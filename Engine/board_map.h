#ifndef _POSTPROCESS
#define _POSTPROCESS

#include "common_struct.h"
#include "image_filter.h"
#include "image_fliter_shader.h"
#include "mesh.h"

namespace dx11 {

using DirectX::SimpleMath::Vector4;
using Microsoft::WRL::ComPtr;

class BoardMap {
  public:
    void
    Initialize(ComPtr<ID3D11Device> &device,
               ComPtr<ID3D11DeviceContext> &context,
               const std::vector<ComPtr<ID3D11ShaderResourceView>> &resources,
               const std::vector<ComPtr<ID3D11RenderTargetView>> &targets,
               const int width, const int height, const int bloomLevels);

    void Render(ComPtr<ID3D11DeviceContext> &context);

    void RenderImageFilter(ComPtr<ID3D11DeviceContext> &context,
                           const ImageFilter &imageFilter);

    void CreateBuffer(ComPtr<ID3D11Device> &device,
                      ComPtr<ID3D11DeviceContext> &context, int width,
                      int height, ComPtr<ID3D11ShaderResourceView> &srv,
                      ComPtr<ID3D11RenderTargetView> &rtv);

  public:
    ImageFilter m_combineFilter;
    std::vector<ImageFilter> m_bloomDownFilters;
    std::vector<ImageFilter> m_bloomUpFilters;
    std::shared_ptr<Mesh> m_mesh;

  private:
    ComPtr<ID3D11InputLayout> m_inputLayout;
    ComPtr<ID3D11SamplerState> m_samplerState;
    ComPtr<ID3D11RasterizerState> m_rasterizerSate;
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_combinePixelShader;
    ComPtr<ID3D11PixelShader> m_bloomDownPixelShader;
    ComPtr<ID3D11PixelShader> m_bloomUpPixelShader;

    std::vector<ComPtr<ID3D11ShaderResourceView>> m_bloomSRVs;
    std::vector<ComPtr<ID3D11RenderTargetView>> m_bloomRTVs;

    std::shared_ptr<ImageFilterShader> image_filter_shader;
};
} // namespace Engine
#endif
