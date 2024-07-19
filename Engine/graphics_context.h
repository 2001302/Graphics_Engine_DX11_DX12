#ifndef _DIRECT3D
#define _DIRECT3D

#include "env.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxtk/DDSTextureLoader.h>
#include <directxtk/SimpleMath.h>
#include <iostream>
#include <windows.h>
#include <wrl.h>
#include <memory>
#include <string>

using namespace DirectX;

namespace dx11 {
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;
using Microsoft::WRL::ComPtr;

inline void ThrowIfFailed(HRESULT hr) {
    if (FAILED(hr)) {
        throw std::exception();
    }
}

class GraphicsContext {
  public:
    static GraphicsContext &Instance() {
        static GraphicsContext instance;
        return instance;
    }
    bool Initialize();
    void BeginScene(float red, float green, float blue, float alpha,
                    bool draw_as_wire);
    void EndScene();

    void SetViewPort(float x, float y, float width, float height);
    bool CreateBuffer();
    void CreateDepthBuffer(ComPtr<ID3D11Device> &device, int screenWidth,
                           int screenHeight, UINT numQualityLevels,
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
    GraphicsContext()
        : swap_chain_(0), device_(0), device_context_(0),
          viewport_(D3D11_VIEWPORT()) {}

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
} // namespace dx11
#endif
