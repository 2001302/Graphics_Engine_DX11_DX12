#include "graphics_util.h"

#include <algorithm>
#include <dxgi.h>    // DXGIFactory
#include <dxgi1_4.h> // DXGIFactory4
#include <fp16.h>
#include <iostream>

namespace graphics {

using namespace std;
using namespace DirectX;

void CheckResult(HRESULT hr, ID3DBlob *errorBlob) {
    if (FAILED(hr)) {
        if ((hr & D3D11_ERROR_FILE_NOT_FOUND) != 0) {
            std::cout << "File not found." << std::endl;
        }
        if (errorBlob) {
            std::cout << "Shader compile error\n"
                      << (char *)errorBlob->GetBufferPointer() << std::endl;
        }
    }
}

void Util::CreateVertexShader(
    ComPtr<ID3D12Device> &device, std::wstring filename,
    ComPtr<ID3DBlob> &m_vertexShader,
    const std::vector<D3D_SHADER_MACRO> shaderMacros) {

    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DCompileFromFile(
        filename.c_str(), shaderMacros.empty() ? NULL : shaderMacros.data(),
        D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", compileFlags, 0,
        &m_vertexShader, &errorBlob);
    CheckResult(hr, errorBlob.Get());
}

void Util::CreatePixelShader(ComPtr<ID3D12Device> &device,
                             std::wstring filename, ComPtr<ID3DBlob> &shader) {
    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DCompileFromFile(
        filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
        "ps_5_0", compileFlags, 0, &shader, &errorBlob);
    CheckResult(hr, errorBlob.Get());
}

void Util::CreateComputeShader(ComPtr<ID3D12Device> &device,
                               std::wstring filename,
                               ComPtr<ID3DBlob> &shader) {
    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DCompileFromFile(
        filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
        "cs_5_0", compileFlags, 0, &shader, &errorBlob);
    CheckResult(hr, errorBlob.Get());
}
} // namespace dx12
