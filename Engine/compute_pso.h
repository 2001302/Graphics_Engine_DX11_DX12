#pragma once

#include "graphics_util.h"

namespace engine {

class ComputePSO {
  public:
    void operator=(const ComputePSO &pso) {
        compute_shader = pso.compute_shader;
    };

  public:
    ComPtr<ID3D11ComputeShader> compute_shader;
};

} // namespace engine