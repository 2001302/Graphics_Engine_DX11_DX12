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
        : vsync_enabled_(false), swap_chain_(0), device_(0), device_context_(0),
          viewport_(D3D11_VIEWPORT()), render_target_view_(0) {}
    Direct3D(const Direct3D &x)
        : vsync_enabled_(x.vsync_enabled_), swap_chain_(x.swap_chain_),
          device_(x.device_), device_context_(x.device_context_),
          viewport_(x.viewport_), render_target_view_(0) {}
    Direct3D &operator=(const Direct3D &ref) {}
    ~Direct3D() {}

    bool vsync_enabled_;
    UINT num_quality_levels_ = 0;

    ComPtr<ID3D11Device> device_;
    ComPtr<IDXGISwapChain> swap_chain_;
    ComPtr<ID3D11DeviceContext> device_context_;

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

    void BeginScene(float, float, float, float);
    void EndScene();

    ComPtr<ID3D11Device> GetDevice();
    ComPtr<ID3D11DeviceContext> GetDeviceContext();

    ComPtr<ID3D11Texture2D> depth_stencil_buffer_;
    ComPtr<ID3D11DepthStencilState> depth_stencil_state_;
    ComPtr<ID3D11DepthStencilView> depth_stencil_view_;

    ComPtr<ID3D11RasterizerState> solid_rasterizer_state_;
    ComPtr<ID3D11RasterizerState> wire_rasterizer_state_;

    ID3D11RenderTargetView *render_target_view_;
    D3D11_VIEWPORT viewport_;
};
} // namespace Engine
#endif