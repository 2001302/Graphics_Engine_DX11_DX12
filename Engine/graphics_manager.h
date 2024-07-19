#ifndef _GRAPHICSMANAGER
#define _GRAPHICSMANAGER

#include "graphics_util.h"

namespace engine {
class GraphicsManager {
  public:
    static GraphicsManager &Instance() {
        static GraphicsManager instance;
        return instance;
    }
    bool Initialize();
    void BeginScene(bool draw_as_wire);
    void EndScene();
    void SetViewPort(float x, float y, float width, float height);
    bool CreateBuffer();

    bool useMSAA = true;
    UINT num_quality_levels = 0;

    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> device_context;
    ComPtr<IDXGISwapChain> swap_chain;
    ComPtr<ID3D11RenderTargetView> back_buffer_RTV;

    ComPtr<ID3D11Texture2D> float_buffer;
    ComPtr<ID3D11Texture2D> resolved_buffer;
    ComPtr<ID3D11RenderTargetView> float_RTV;
    ComPtr<ID3D11RenderTargetView> resolved_RTV;
    ComPtr<ID3D11ShaderResourceView> float_SRV;
    ComPtr<ID3D11ShaderResourceView> resolved_SRV;

    ComPtr<ID3D11RasterizerState> solid_rasterizer_state;
    ComPtr<ID3D11RasterizerState> wire_rasterizer_state;

    ComPtr<ID3D11DepthStencilState> depth_stencil_state;
    ComPtr<ID3D11DepthStencilView> depth_stencil_view;

    D3D11_VIEWPORT viewport;

  private:
    GraphicsManager()
        : swap_chain(0), device(0), device_context(0),
          viewport(D3D11_VIEWPORT()) {}
    void CreateDepthBuffer();
};

} // namespace dx11
#endif
