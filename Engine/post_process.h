#pragma once

#include "compute_pso.h"
#include "constant_buffer.h"
#include "env.h"
#include "image_filter.h"
#include "mesh.h"

namespace engine {
class PostProcess {
  public:
    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context);

    void Render(ComPtr<ID3D11Device> &device,
                ComPtr<ID3D11DeviceContext> &context,
                GlobalConstants *constsCPU, ComPtr<ID3D11Buffer> constsGPU);
    void RenderImageFilter(ComPtr<ID3D11DeviceContext> &context,
                           const ImageFilter &imageFilter);

  private:
    ImageFilter m_combineFilter;
    std::shared_ptr<Mesh> m_mesh;
    // ComPtr<ID3D11ComputeShader> combine_CS;
    // ComputePSO compute_PSO;
};
} // namespace engine