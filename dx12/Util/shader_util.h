#ifndef _SHADER_UTIL
#define _SHADER_UTIL

#include "../gpu/core/device_manager.h"
#include "../gpu/pso/pipeline_state_object.h"
#include "../gpu/buffer/sampler_state.h"

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
