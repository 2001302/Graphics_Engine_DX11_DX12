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

    bool vsync_enabled_;
    UINT num_quality_levels_ = 0;

    ComPtr<ID3D11Device> device_;
    ComPtr<ID3D11DeviceContext> device_context_;
    ComPtr<IDXGISwapChain> swap_chain_;
    ComPtr<ID3D11Texture2D> depth_stencil_buffer_;
    ComPtr<ID3D11DepthStencilState> depth_stencil_state_;
    ComPtr<ID3D11DepthStencilView> depth_stencil_view_;
    ComPtr<ID3D11RasterizerState> solid_rasterizer_state_;
    ComPtr<ID3D11RasterizerState> wire_rasterizer_state_;
    ID3D11RenderTargetView *render_target_view_;
    D3D11_VIEWPORT viewport_;

  public:
    static Direct3D &GetInstance() {
        static Direct3D instance;
        return instance;
    }
    bool Initialize(Env *env, bool vsync, HWND main_window, bool fullscreen);
    void BeginScene(float red, float green, float blue, float alpha);
    void EndScene();

    void SetViewPort(float x, float y, float width, float height);
    bool CreateRenderTargetView();
    bool CreateDepthBuffer(Env *env);

    ComPtr<ID3D11Device> device();
    ComPtr<ID3D11DeviceContext> device_context();
    ComPtr<IDXGISwapChain> swap_chain();
    ComPtr<ID3D11Texture2D> depth_stencil_buffer();
    ComPtr<ID3D11DepthStencilState> depth_stencil_state();
    ComPtr<ID3D11DepthStencilView> depth_stencil_view();
    ComPtr<ID3D11RasterizerState> solid_rasterizer_state();
    ComPtr<ID3D11RasterizerState> wire_rasterizer_state();
    ID3D11RenderTargetView **render_target_view();
    D3D11_VIEWPORT viewport();
};
} // namespace Engine
#endif