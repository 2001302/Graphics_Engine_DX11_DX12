#pragma once

#include "image_filter.h"
#include "mesh.h"

namespace engine {
class PostProcess {
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
    std::vector<ComPtr<ID3D11ShaderResourceView>> m_bloomSRVs;
    std::vector<ComPtr<ID3D11RenderTargetView>> m_bloomRTVs;
};
} // namespace hlab