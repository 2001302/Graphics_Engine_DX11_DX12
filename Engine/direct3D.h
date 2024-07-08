#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_

#include <directxtk/DDSTextureLoader.h> 

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

    void CreatePixelShader(const std::wstring &filename,
                           ComPtr<ID3D11PixelShader> &pixelShader) {
        ComPtr<ID3DBlob> shaderBlob;
        ComPtr<ID3DBlob> errorBlob;

        UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
        compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        HRESULT hr = D3DCompileFromFile(
            filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
            "ps_5_0", compileFlags, 0, &shaderBlob, &errorBlob);

        Direct3D::GetInstance().device()->CreatePixelShader(
            shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL,
            &pixelShader);
    }

    void CreateVertexShaderAndInputLayout(
        const std::wstring &filename,
        const std::vector<D3D11_INPUT_ELEMENT_DESC> &inputElements,
        ComPtr<ID3D11VertexShader> &vertexShader,
        ComPtr<ID3D11InputLayout> &inputLayout) {
        ComPtr<ID3DBlob> shaderBlob;
        ComPtr<ID3DBlob> errorBlob;

        UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
        compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        HRESULT hr = D3DCompileFromFile(
            filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
            "vs_5_0", compileFlags, 0, &shaderBlob, &errorBlob);

        Direct3D::GetInstance().device()->CreateVertexShader(
            shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL,
            &vertexShader);

        Direct3D::GetInstance().device()->CreateInputLayout(
            inputElements.data(), UINT(inputElements.size()),
            shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
            &inputLayout);
    }

    template <typename T_CONSTANT>
    void CreateConstantBuffer(const T_CONSTANT &constantBufferData,
                              ComPtr<ID3D11Buffer> &constantBuffer) {
        // For a constant buffer (BindFlags of D3D11_BUFFER_DESC set to
        // D3D11_BIND_CONSTANT_BUFFER), you must set the ByteWidth value of
        // D3D11_BUFFER_DESC in multiples of 16, and less than or equal to
        // D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT.

        D3D11_BUFFER_DESC cbDesc;
        cbDesc.ByteWidth = sizeof(constantBufferData);
        cbDesc.Usage = D3D11_USAGE_DYNAMIC;
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        cbDesc.MiscFlags = 0;
        cbDesc.StructureByteStride = 0;

        // Fill in the subresource data.
        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = &constantBufferData;
        initData.SysMemPitch = 0;
        initData.SysMemSlicePitch = 0;

        auto hr = Direct3D::GetInstance().device()->CreateBuffer(
            &cbDesc, &initData, constantBuffer.GetAddressOf());
        if (FAILED(hr)) {
            std::cout << "CreateConstantBuffer() CreateBuffer failed()."
                      << std::endl;
        }
    }
    template <typename T_DATA>
    void UpdateBuffer(const T_DATA &bufferData, ComPtr<ID3D11Buffer> &buffer) {

        if (!buffer) {
            std::cout << "UpdateBuffer() buffer was not initialized."
                      << std::endl;
        }

        D3D11_MAPPED_SUBRESOURCE ms;
        Direct3D::GetInstance().device_context()->Map(
            buffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
        memcpy(ms.pData, &bufferData, sizeof(bufferData));
        Direct3D::GetInstance().device_context()->Unmap(buffer.Get(), NULL);
    }
    template <typename T_VERTEX>
    void CreateVertexBuffer(const std::vector<T_VERTEX> &vertices,
                            ComPtr<ID3D11Buffer> &vertexBuffer) {

        // D3D11_USAGE enumeration (d3d11.h)
        // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_usage

        D3D11_BUFFER_DESC bufferDesc;
        ZeroMemory(&bufferDesc, sizeof(bufferDesc));
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.ByteWidth = UINT(sizeof(T_VERTEX) * vertices.size());
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
        bufferDesc.StructureByteStride = sizeof(T_VERTEX);

        D3D11_SUBRESOURCE_DATA vertexBufferData = {0};
        vertexBufferData.pSysMem = vertices.data();
        vertexBufferData.SysMemPitch = 0;
        vertexBufferData.SysMemSlicePitch = 0;

        const HRESULT hr = Direct3D::GetInstance().device()->CreateBuffer(
            &bufferDesc, &vertexBufferData, vertexBuffer.GetAddressOf());
        if (FAILED(hr)) {
            std::cout << "CreateBuffer() failed. " << std::hex << hr
                      << std::endl;
        };
    }
    void CreateIndexBuffer(const std::vector<uint32_t> &indices,
                           ComPtr<ID3D11Buffer> &indexBuffer) {
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.ByteWidth = UINT(sizeof(int) * indices.size());
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
        bufferDesc.StructureByteStride = sizeof(int);

        D3D11_SUBRESOURCE_DATA indexBufferData = {0};
        indexBufferData.pSysMem = indices.data();
        indexBufferData.SysMemPitch = 0;
        indexBufferData.SysMemSlicePitch = 0;

        Direct3D::GetInstance().device()->CreateBuffer(
            &bufferDesc, &indexBufferData, indexBuffer.GetAddressOf());
    }
    void CreateGeometryShader(const std::wstring &filename,
                              ComPtr<ID3D11GeometryShader> &geometryShader) {

        ComPtr<ID3DBlob> shaderBlob;
        ComPtr<ID3DBlob> errorBlob;

        UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
        compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        // 쉐이더의 시작점의 이름이 "main"인 함수로 지정
        // D3D_COMPILE_STANDARD_FILE_INCLUDE 추가: 쉐이더에서 include 사용
        HRESULT hr = D3DCompileFromFile(
            filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
            "gs_5_0", compileFlags, 0, &shaderBlob, &errorBlob);

        // CheckResult(hr, errorBlob.Get());

        Direct3D::GetInstance().device()->CreateGeometryShader(
            shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL,
            &geometryShader);
    }

void CreateDDSTexture(const wchar_t *filename,
                      ComPtr<ID3D11ShaderResourceView> &texResView);

    ComPtr<ID3D11Device> device();
    ComPtr<ID3D11DeviceContext> device_context();
    ComPtr<IDXGISwapChain> swap_chain();
    ComPtr<ID3D11Texture2D> float_buffer();
    ComPtr<ID3D11DepthStencilState> depth_stencil_state();
    ComPtr<ID3D11DepthStencilView> depth_stencil_view();
    ComPtr<ID3D11RasterizerState> solid_rasterizer_state();
    ComPtr<ID3D11RasterizerState> wire_rasterizer_state();
    ComPtr<ID3D11RenderTargetView> float_RTV();
    D3D11_VIEWPORT viewport();
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
    ComPtr<ID3D11RenderTargetView> float_RTV_;
    ComPtr<ID3D11RenderTargetView> resolved_RTV_;
    ComPtr<ID3D11ShaderResourceView> float_SRV_;
    ComPtr<ID3D11ShaderResourceView> resolved_SRV_;

    ComPtr<ID3D11RasterizerState> solid_rasterizer_state_;
    ComPtr<ID3D11RasterizerState> wire_rasterizer_state_;

    ComPtr<ID3D11DepthStencilState> depth_stencil_state_;
    ComPtr<ID3D11DepthStencilView> depth_stencil_view_;

    D3D11_VIEWPORT viewport_;
};
} // namespace Engine
#endif