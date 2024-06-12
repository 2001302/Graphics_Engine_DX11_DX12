#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_

#include "common_struct.h"
#include "env.h"

using namespace DirectX;

namespace Engine {
using Microsoft::WRL::ComPtr;

class Direct3D {
  private:
    Direct3D()
        : vsync_enabled(false), swap_chain(0), device(0), device_context(0),
          viewport(D3D11_VIEWPORT()) {}
    Direct3D(const Direct3D &x)
        : vsync_enabled(x.vsync_enabled), swap_chain(x.swap_chain),
          device(x.device), device_context(x.device_context),
          viewport(x.viewport) {}
    Direct3D &operator=(const Direct3D &ref) {}
    ~Direct3D() {}

    bool vsync_enabled;
    UINT num_quality_levels = 0;

    ComPtr<ID3D11Device> device;
    ComPtr<IDXGISwapChain> swap_chain;
    ComPtr<ID3D11DeviceContext> device_context;

  public:
    static Direct3D &GetInstance() {
        static Direct3D instance;
        return instance;
    }
    bool Init(Env *env, bool vsync, HWND hwnd, bool fullscreen);
    bool InitDirect3D(int screenWidth, int screenHeight, HWND hwnd,
                      bool fullscreen);
    bool InitMainScene(int screenWidth, int screenHeight);

    void SetViewPort(float x, float y, float width, float height);
    void Shutdown();

    void BeginScene(float, float, float, float);
    void EndScene();

    ComPtr<ID3D11Device> GetDevice();
    ComPtr<ID3D11DeviceContext> GetDeviceContext();

    ComPtr<ID3D11Texture2D> depth_stencil_buffer;
    ComPtr<ID3D11DepthStencilState> depth_stencil_state;
    ComPtr<ID3D11DepthStencilView> depth_stencil_view;

    ComPtr<ID3D11RasterizerState> solid_rasterizer_state;
    ComPtr<ID3D11RasterizerState> wire_rasterizer_state;

    ID3D11RenderTargetView *render_target_view;
    D3D11_VIEWPORT viewport;
};
} // namespace Engine
#endif