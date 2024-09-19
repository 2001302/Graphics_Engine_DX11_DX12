#ifndef _SHADER_UTIL
#define _SHADER_UTIL

#include "graphics_core.h"
#include "graphics_pso.h"
#include "sampler_state.h"

namespace graphics {
class ShaderUtil {
  public:
    static void
    CreateVertexShader(ComPtr<ID3D12Device> device, std::wstring filename,
                       ComPtr<ID3DBlob> &m_vertexShader,
                       const std::vector<D3D_SHADER_MACRO> shaderMacros = {
                           /* Empty default */});

    static void CreatePixelShader(ComPtr<ID3D12Device> device,
                                  std::wstring filename,
                                  ComPtr<ID3DBlob> &shader);

    static void CreateComputeShader(ComPtr<ID3D12Device> device,
                                    std::wstring filename,
                                    ComPtr<ID3DBlob> &shader);
};
} // namespace graphics
#endif
