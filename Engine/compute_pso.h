#pragma once

#include "graphics_util.h"

namespace engine {

class ComputePSO {
  public:
    void operator=(const ComputePSO &pso) {
        m_computeShader = pso.m_computeShader;
    };

  public:
    ComPtr<ID3D11ComputeShader> m_computeShader;
};

} // namespace engine