#ifndef _DIRECT3D
#define _DIRECT3D

#define MAX_LIGHTS 3
#define LIGHT_OFF 0x00
#define LIGHT_DIRECTIONAL 0x01
#define LIGHT_POINT 0x02
#define LIGHT_SPOT 0x04
#define LIGHT_SHADOW 0x10

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

class GraphicsManager {
  public:
    static GraphicsManager &Instance() {
        static GraphicsManager instance;
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
    ComPtr<ID3D11DepthStencilState> depth_stencil_state();
    ComPtr<ID3D11DepthStencilView> depth_stencil_view();
    ComPtr<ID3D11RasterizerState> solid_rasterizer_state();
    ComPtr<ID3D11RasterizerState> wire_rasterizer_state();
    D3D11_VIEWPORT viewport();

    ComPtr<ID3D11RenderTargetView> back_buffer_RTV();

    ComPtr<ID3D11Texture2D> float_buffer();
    ComPtr<ID3D11Texture2D> resolved_buffer();

    ComPtr<ID3D11RenderTargetView> float_RTV();
    ComPtr<ID3D11RenderTargetView> resolved_RTV();

    ComPtr<ID3D11ShaderResourceView> float_SRV();
    ComPtr<ID3D11ShaderResourceView> resolved_SRV();

    ComPtr<ID3D11Texture2D> post_effects_buffer();
    ComPtr<ID3D11RenderTargetView> post_effects_RTV();
    ComPtr<ID3D11ShaderResourceView> post_effects_SRV();
  private:
    GraphicsManager()
        : swap_chain_(0), device_(0), device_context_(0),
          viewport_(D3D11_VIEWPORT()) {}

    ComPtr<ID3D11Device> device_;
    ComPtr<ID3D11DeviceContext> device_context_;
    ComPtr<IDXGISwapChain> swap_chain_;

    ComPtr<ID3D11RenderTargetView> back_buffer_RTV_;

    ComPtr<ID3D11Texture2D> float_buffer_;
    ComPtr<ID3D11Texture2D> resolved_buffer_;

    ComPtr<ID3D11RenderTargetView> float_RTV_;
    ComPtr<ID3D11RenderTargetView> resolved_RTV_;

    ComPtr<ID3D11ShaderResourceView> float_SRV_;
    ComPtr<ID3D11ShaderResourceView> resolved_SRV_;

    ComPtr<ID3D11Texture2D> post_effects_buffer_;
    ComPtr<ID3D11RenderTargetView> post_effects_RTV_;
    ComPtr<ID3D11ShaderResourceView> post_effects_SRV_;

    ComPtr<ID3D11RasterizerState> solid_rasterizer_state_;
    ComPtr<ID3D11RasterizerState> wire_rasterizer_state_;

    ComPtr<ID3D11DepthStencilState> depth_stencil_state_;
    ComPtr<ID3D11DepthStencilView> depth_stencil_view_;
    ComPtr<ID3D11DepthStencilView> depth_only_DSV_;
    ComPtr<ID3D11ShaderResourceView> depth_only_SRV_;

    D3D11_VIEWPORT viewport_;

    // Shadow maps
    int shadow_width = 1280;
    int shadow_height = 1280;
    ComPtr<ID3D11Texture2D> shadow_buffers[MAX_LIGHTS]; // No MSAA
    ComPtr<ID3D11DepthStencilView> shadow_DSVs[MAX_LIGHTS];
    ComPtr<ID3D11ShaderResourceView> shadow_SRVs[MAX_LIGHTS];

    bool useMSAA = true;
    UINT num_quality_levels_ = 0;
};
} // namespace dx11
#endif
