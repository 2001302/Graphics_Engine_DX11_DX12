#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_

#include "common_struct.h"
#include "env.h"

using namespace DirectX;

namespace Engine {
using Microsoft::WRL::ComPtr;

class Direct3D {
  public:
    static Direct3D &GetInstance() {
        static Direct3D instance;
        return instance;
    }
    bool Initialize(Env *env, bool vsync, HWND main_window, bool fullscreen);
    void BeginScene(float red, float green, float blue, float alpha);
    void EndScene();

    void SetViewPort(float x, float y, float width, float height);
    bool CreateBuffer(Env *env);
    void CreateDepthBuffer(
        ComPtr<ID3D11Device> &device, int screenWidth, int screenHeight,
        UINT numQualityLevels,
        ComPtr<ID3D11DepthStencilView> &depthStencilView);

    ComPtr<ID3D11Device> device();
    ComPtr<ID3D11DeviceContext> device_context();
    ComPtr<IDXGISwapChain> swap_chain();
    ComPtr<ID3D11Texture2D> depth_stencil_buffer();
    ComPtr<ID3D11DepthStencilState> depth_stencil_state();
    ComPtr<ID3D11DepthStencilView> depth_stencil_view();
    ComPtr<ID3D11RasterizerState> solid_rasterizer_state();
    ComPtr<ID3D11RasterizerState> wire_rasterizer_state();
    ComPtr<ID3D11RenderTargetView> render_target_view();
    D3D11_VIEWPORT viewport();
    ComPtr<ID3D11Texture2D> float_buffer();
    ComPtr<ID3D11Texture2D> resolved_buffer();
    ComPtr<ID3D11RenderTargetView> back_buffer_RTV();
    ComPtr<ID3D11ShaderResourceView> resolved_SRV();

  private:
    Direct3D()
        : vsync_enabled_(true), swap_chain_(0), device_(0), device_context_(0),
          viewport_(D3D11_VIEWPORT()){}

    bool vsync_enabled_;
    bool useMSAA = true;
    UINT num_quality_levels_ = 0;

    ComPtr<ID3D11Device> device_;
    ComPtr<ID3D11DeviceContext> device_context_;
    ComPtr<IDXGISwapChain> swap_chain_;
    ComPtr<ID3D11RenderTargetView> back_buffer_RTV_;

    // 삼각형 레스터화 -> float(MSAA) -> resolved(No MSAA)
    // -> 후처리(블룸, 톤매핑) -> backBuffer(최종 SwapChain Present)
    ComPtr<ID3D11Texture2D> float_buffer_;
    ComPtr<ID3D11Texture2D> resolved_buffer_;
    ComPtr<ID3D11RenderTargetView> float_RTV;
    ComPtr<ID3D11RenderTargetView> resolved_RTV;
    ComPtr<ID3D11ShaderResourceView> float_SRV;
    ComPtr<ID3D11ShaderResourceView> resolved_SRV_;

    ComPtr<ID3D11RasterizerState> solid_rasterizer_state_;
    ComPtr<ID3D11RasterizerState> wire_rasterizer_state_;

    ComPtr<ID3D11DepthStencilState> depth_stencil_state_;
    ComPtr<ID3D11DepthStencilView> depth_stencil_view_;

    D3D11_VIEWPORT viewport_;
};
} // namespace Engine
#endif