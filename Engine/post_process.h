#pragma once

#include "image_filter.h"
#include "mesh.h"
#include "compute_pso.h"
#include "constant_buffer.h"

namespace engine {
class PostProcess {
  public:
    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context);

    void Render(ComPtr<ID3D11Device> &device,
                ComPtr<ID3D11DeviceContext> &context, GlobalConstants* constsCPU,
                ComPtr<ID3D11Buffer> constsGPU);

  private:
    ComPtr<ID3D11ComputeShader> combine_CS;
    ComputePSO compute_PSO;
};
} // namespace engine