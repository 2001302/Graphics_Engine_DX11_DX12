#ifndef _GRAPHICSMANAGER
#define _GRAPHICSMANAGER

#include <structure/env.h>
#include "graphics_common.h"

namespace graphics {
class GpuCore {
  public:
    static GpuCore &Instance() {
        static GpuCore instance;
        return instance;
    }
    bool Initialize();
    bool CreateBuffer();

    void SetMainViewport();

    bool useMSAA = true;
    UINT num_quality_levels = 0;

    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> device_context;

    ComPtr<IDXGISwapChain> swap_chain;
    ComPtr<ID3D11RenderTargetView> back_buffer_RTV;

    ComPtr<ID3D11Texture2D> float_buffer;
    ComPtr<ID3D11RenderTargetView> float_RTV;

    ComPtr<ID3D11Texture2D> resolved_buffer;
    ComPtr<ID3D11RenderTargetView> resolved_RTV;
    ComPtr<ID3D11ShaderResourceView> resolved_SRV;
    ComPtr<ID3D11UnorderedAccessView> resolved_UAV;

    ComPtr<ID3D11DepthStencilView> depth_stencil_view;

    D3D11_VIEWPORT viewport;

  private:
    GpuCore()
        : swap_chain(0), device(0), device_context(0),
          viewport(D3D11_VIEWPORT()) {}
    void CreateDepthBuffer();
};

} // namespace graphics
#endif
