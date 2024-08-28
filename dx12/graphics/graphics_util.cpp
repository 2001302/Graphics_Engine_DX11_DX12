#include "graphics_util.h"

#include <algorithm>
#include <dxgi.h>    // DXGIFactory
#include <dxgi1_4.h> // DXGIFactory4
#include <fp16.h>
#include <iostream>

namespace dx12 {

using namespace std;
using namespace DirectX;

//
//void Util::CreateMetallicRoughnessTexture(
//    const std::string metallicFilename, const std::string roughnessFilename,
//    ComPtr<ID3D12Resource> &texture,
//    ComPtr<ID3D12GraphicsCommandList> command_list) {
//
//    if (!metallicFilename.empty() && (metallicFilename == roughnessFilename)) {
//        Util::CreateTexture(metallicFilename, false, texture, command_list,
//                            textureUploadHeap);
//    } else {
//        int mWidth = 0, mHeight = 0;
//        int rWidth = 0, rHeight = 0;
//        std::vector<uint8_t> mImage;
//        std::vector<uint8_t> rImage;
//
//        if (!metallicFilename.empty()) {
//            ReadImage(metallicFilename, mImage, mWidth, mHeight);
//        }
//
//        if (!roughnessFilename.empty()) {
//            ReadImage(roughnessFilename, rImage, rWidth, rHeight);
//        }
//
//        if (!metallicFilename.empty() && !roughnessFilename.empty()) {
//            assert(mWidth == rWidth);
//            assert(mHeight == rHeight);
//        }
//
//        vector<uint8_t> combinedImage(size_t(mWidth * mHeight) * 4);
//        fill(combinedImage.begin(), combinedImage.end(), 0);
//
//        for (size_t i = 0; i < size_t(mWidth * mHeight); i++) {
//            if (rImage.size())
//                combinedImage[4 * i + 1] = rImage[4 * i]; // Green = Roughness
//            if (mImage.size())
//                combinedImage[4 * i + 2] = mImage[4 * i]; // Blue = Metalness
//        }
//
//        CreateTextureHelper(mWidth, mHeight, combinedImage,
//                            DXGI_FORMAT_R8G8B8A8_UNORM, texture, command_list,
//                            textureUploadHeap);
//    }
//}

// void Util::CreateDDSTexture(const wchar_t *filename, bool isCubeMap,
//    ComPtr<ID3D11ShaderResourceView> &textureResourceView) {

//    ComPtr<ID3D12Resource> texture;

//    UINT miscFlags = 0;
//    if (isCubeMap) {
//        miscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
//    }

//    // https://github.com/microsoft/DirectXTK/wiki/DDSTextureLoader
//    ThrowIfFailed(CreateDDSTextureFromFileEx(
//        GpuCore::Instance().device.Get(), filename, 0, D3D11_USAGE_DEFAULT,
//        D3D11_BIND_SHADER_RESOURCE, 0, miscFlags, DDS_LOADER_FLAGS(false),
//        (ID3D11Resource **)texture.GetAddressOf(),
//        textureResourceView.GetAddressOf(), NULL));
//}

ComPtr<ID3D12Resource>
CreateStagingTexture(const int width, const int height,
                     const std::vector<uint8_t> &image,
                     const DXGI_FORMAT pixelFormat = DXGI_FORMAT_R8G8B8A8_UNORM,
                     const int mipLevels = 1, const int arraySize = 1) {

    ComPtr<ID3D12Resource> stagingTexture;

    // 리소스 설명자 생성
    D3D12_RESOURCE_DESC txtDesc;
    ZeroMemory(&txtDesc, sizeof(txtDesc));
    txtDesc.Width = width;
    txtDesc.Height = height;
    txtDesc.MipLevels = mipLevels;
    txtDesc.Format = pixelFormat;
    txtDesc.SampleDesc.Count = 1;
    txtDesc.SampleDesc.Quality = 0;
    txtDesc.DepthOrArraySize = arraySize;
    txtDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    txtDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    // 리소스 할당
    CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_READBACK);
    ThrowIfFailed(dx12::GpuCore::Instance().device->CreateCommittedResource(
        &heapProperties, D3D12_HEAP_FLAG_NONE, &txtDesc,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
        IID_PPV_ARGS(stagingTexture.GetAddressOf())));

    // 텍스처 매핑
    D3D12_RANGE readRange = {0, 0};
    uint8_t *mappedData = nullptr;
    ThrowIfFailed(stagingTexture->Map(0, &readRange,
                                      reinterpret_cast<void **>(&mappedData)));

    // 데이터 복사
    const uint8_t *srcData = image.data();
    for (UINT row = 0; row < height; ++row) {
        memcpy(mappedData + row * width * 4, srcData + row * width * 4,
               width * 4);
    }

    // 매핑 해제
    D3D12_RANGE writeRange = {0, width * height * 4}; // 쓰기 범위 설정
    stagingTexture->Unmap(0, &writeRange);

    return stagingTexture;
}

// ComPtr<ID3D11Texture3D>
// Util::CreateStagingTexture3D(const int width, const int height, const int
// depth,
//                             const DXGI_FORMAT pixelFormat) {
//
//    D3D11_TEXTURE3D_DESC txtDesc;
//    ZeroMemory(&txtDesc, sizeof(txtDesc));
//    txtDesc.Width = width;
//    txtDesc.Height = height;
//    txtDesc.Depth = depth;
//    txtDesc.MipLevels = 1;
//    txtDesc.Format = pixelFormat;
//    txtDesc.Usage = D3D11_USAGE_STAGING;
//    txtDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
//
//    ComPtr<ID3D11Texture3D> stagingTexture;
//    if (FAILED(GpuCore::Instance().device->CreateTexture3D(
//            &txtDesc, NULL, stagingTexture.GetAddressOf()))) {
//        cout << "CreateStagingTexture3D() failed." << endl;
//    }
//
//    return stagingTexture;
//}

//  void Util::WriteToFile(
//                                ComPtr<ID3D11Texture2D> &textureToWrite,
//                                const std::string filename) {
//
//     D3D11_TEXTURE2D_DESC desc;
//     textureToWrite->GetDesc(&desc);
//     desc.SampleDesc.Count = 1;
//     desc.SampleDesc.Quality = 0;
//     desc.BindFlags = 0;
//     desc.MiscFlags = 0;
//     desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
//     媛?? desc.Usage = D3D11_USAGE_STAGING;
//
//     ComPtr<ID3D11Texture2D> stagingTexture;
//     if (FAILED(GpuCore::Instance().device->CreateTexture2D(
//             &desc, NULL,
//                                        stagingTexture.GetAddressOf()))) {
//         cout << "Failed()" << endl;
//     }
//
//     // context->CopyResource(stagingTexture.Get(), pTemp.Get());
//
//     D3D11_BOX box;
//     box.left = 0;
//     box.right = desc.Width;
//     box.top = 0;
//     box.bottom = desc.Height;
//     box.front = 0;
//     box.back = 1;
//     GpuCore::Instance().device_context->CopySubresourceRegion(
//         stagingTexture.Get(), 0, 0, 0, 0,
//                                    textureToWrite.Get(), 0, &box);
//
//     std::vector<uint8_t> pixels(desc.Width * desc.Height * 4);
//
//     D3D11_MAPPED_SUBRESOURCE ms;
//     GpuCore::Instance().device_context->Map(stagingTexture.Get(), NULL,
//                                                  D3D11_MAP_READ, NULL,
//                  &ms); // D3D11_MAP_READ 二쇱쓽
//
//     // ms.RowPitch媛 width * sizeof(uint8_t) * 4蹂대떎 ???섎룄
//     ?덉뼱??
//     // for臾몄쑝濡?媛濡쒖쨪 ?섎굹??蹂듭궗
//     uint8_t *pData = (uint8_t *)ms.pData;
//     for (unsigned int h = 0; h < desc.Height; h++) {
//         memcpy(&pixels[h * desc.Width * 4], &pData[h * ms.RowPitch],
//                desc.Width * sizeof(uint8_t) * 4);
//     }
//
//     GpuCore::Instance().device_context->Unmap(stagingTexture.Get(), NULL);
//
//     stbi_write_png(filename.c_str(), desc.Width, desc.Height, 4,
//     pixels.data(),
//                    desc.Width * 4);
//
//     cout << filename << endl;
// }
//
//  void Util::CreateUATexture(
//                                    const int width, const int height,
//                                    const DXGI_FORMAT pixelFormat,
//                                    ComPtr<ID3D11Texture2D> &texture,
//                                    ComPtr<ID3D11RenderTargetView> &rtv,
//                                    ComPtr<ID3D11ShaderResourceView> &srv,
//                                    ComPtr<ID3D11UnorderedAccessView> &uav) {
//
//     D3D11_TEXTURE2D_DESC txtDesc;
//     ZeroMemory(&txtDesc, sizeof(txtDesc));
//     txtDesc.Width = width;
//     txtDesc.Height = height;
//     txtDesc.MipLevels = 1;
//     txtDesc.ArraySize = 1;
//     txtDesc.Format = pixelFormat; // 二쇰줈 FLOAT ?ъ슜
//     txtDesc.SampleDesc.Count = 1;
//     txtDesc.Usage = D3D11_USAGE_DEFAULT;
//     txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET
//     |
//                         D3D11_BIND_UNORDERED_ACCESS;
//     txtDesc.MiscFlags = 0;
//     txtDesc.CPUAccessFlags = 0;
//
//     ThrowIfFailed(GpuCore::Instance().device->CreateTexture2D(
//         &txtDesc, NULL, texture.GetAddressOf()));
//     ThrowIfFailed(GpuCore::Instance().device->CreateRenderTargetView(
//         texture.Get(), NULL,
//                                                  rtv.GetAddressOf()));
//     ThrowIfFailed(GpuCore::Instance().device->CreateShaderResourceView(
//         texture.Get(), NULL,
//                                                    srv.GetAddressOf()));
//     ThrowIfFailed(GpuCore::Instance().device->CreateUnorderedAccessView(
//         texture.Get(), NULL,
//                                                     uav.GetAddressOf()));
// }
//
//  size_t Util::GetPixelSize(DXGI_FORMAT pixelFormat) {
//
//     switch (pixelFormat) {
//     case DXGI_FORMAT_R16G16B16A16_FLOAT:
//         return sizeof(uint16_t) * 4;
//     case DXGI_FORMAT_R32G32B32A32_FLOAT:
//         return sizeof(uint32_t) * 4;
//     case DXGI_FORMAT_R32_FLOAT:
//         return sizeof(uint32_t) * 1;
//     case DXGI_FORMAT_R8G8B8A8_UNORM:
//         return sizeof(uint8_t) * 4;
//     case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
//         return sizeof(uint8_t) * 4;
//     case DXGI_FORMAT_R32_SINT:
//         return sizeof(int32_t) * 1;
//     case DXGI_FORMAT_R16_FLOAT:
//         return sizeof(uint16_t) * 1;
//     }
//
//     cout << "PixelFormat not implemented " << pixelFormat << endl;
//
//     return sizeof(uint8_t) * 4;
// }
//
//  void Util::CreateTexture3D(const int width, const int height,
//     const int depth, const DXGI_FORMAT pixelFormat,
//     const vector<float> &initData, ComPtr<ID3D11Texture3D> &texture,
//     ComPtr<ID3D11RenderTargetView> &rtv, ComPtr<ID3D11ShaderResourceView>
//     &srv, ComPtr<ID3D11UnorderedAccessView> &uav) {
//
//     D3D11_TEXTURE3D_DESC txtDesc;
//     ZeroMemory(&txtDesc, sizeof(txtDesc));
//     txtDesc.Width = width;
//     txtDesc.Height = height;
//     txtDesc.Depth = depth;
//     txtDesc.MipLevels = 1;
//     txtDesc.Format = pixelFormat;
//     txtDesc.Usage = D3D11_USAGE_DEFAULT;
//     txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET
//     |
//                         D3D11_BIND_UNORDERED_ACCESS;
//     txtDesc.MiscFlags = 0;
//     txtDesc.CPUAccessFlags = 0;
//
//     if (initData.size() > 0) {
//         size_t pixelSize = GetPixelSize(pixelFormat);
//         D3D11_SUBRESOURCE_DATA bufferData;
//         ZeroMemory(&bufferData, sizeof(bufferData));
//         bufferData.pSysMem = initData.data();
//         bufferData.SysMemPitch = UINT(width * pixelSize);
//         bufferData.SysMemSlicePitch = UINT(width * height * pixelSize);
//         ThrowIfFailed(GpuCore::Instance().device->CreateTexture3D(
//             &txtDesc, &bufferData,
//                                               texture.GetAddressOf()));
//     } else {
//         ThrowIfFailed(GpuCore::Instance().device->CreateTexture3D(
//             &txtDesc, NULL, texture.GetAddressOf()));
//     }
//
//     ThrowIfFailed(GpuCore::Instance().device->CreateRenderTargetView(
//         texture.Get(), NULL,
//                                                  rtv.GetAddressOf()));
//     ThrowIfFailed(GpuCore::Instance().device->CreateShaderResourceView(
//         texture.Get(), NULL,
//                                                    srv.GetAddressOf()));
//     ThrowIfFailed(GpuCore::Instance().device->CreateUnorderedAccessView(
//         texture.Get(), NULL,
//                                                     uav.GetAddressOf()));
// }
//
//  void Util::CreateStructuredBuffer( const UINT numElements,
//     const UINT sizeElement, const void *initData, ComPtr<ID3D11Buffer>
//     &buffer, ComPtr<ID3D11ShaderResourceView> &srv,
//     ComPtr<ID3D11UnorderedAccessView> &uav) {
//
//     D3D11_BUFFER_DESC bufferDesc;
//     ZeroMemory(&bufferDesc, sizeof(bufferDesc));
//     bufferDesc.Usage = D3D11_USAGE_DEFAULT;
//     bufferDesc.ByteWidth = numElements * sizeElement;
//     bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | // Compute Shader
//                            D3D11_BIND_SHADER_RESOURCE;   // Vertex Shader
//     bufferDesc.StructureByteStride = sizeElement;
//     bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
//
//     // 李멸퀬: Structured??D3D11_BIND_VERTEX_BUFFER濡??ъ슜 遺덇?
//
//     if (initData) {
//         D3D11_SUBRESOURCE_DATA bufferData;
//         ZeroMemory(&bufferData, sizeof(bufferData));
//         bufferData.pSysMem = initData;
//         ThrowIfFailed(GpuCore::Instance().device->CreateBuffer(
//             &bufferDesc, &bufferData,
//                                            buffer.GetAddressOf()));
//     } else {
//         ThrowIfFailed(GpuCore::Instance().device->CreateBuffer(
//             &bufferDesc, NULL, buffer.GetAddressOf()));
//     }
//
//     D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
//     ZeroMemory(&uavDesc, sizeof(uavDesc));
//     uavDesc.Format = DXGI_FORMAT_UNKNOWN;
//     uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
//     uavDesc.Buffer.NumElements = numElements;
//     GpuCore::Instance().device->CreateUnorderedAccessView(
//         buffer.Get(), &uavDesc,
//                                       uav.GetAddressOf());
//
//     D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
//     ZeroMemory(&srvDesc, sizeof(srvDesc));
//     srvDesc.Format = DXGI_FORMAT_UNKNOWN;
//     srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
//     srvDesc.BufferEx.NumElements = numElements;
//     GpuCore::Instance().device->CreateShaderResourceView(
//         buffer.Get(), &srvDesc,
//                                      srv.GetAddressOf());
// }
//
//  void Util::CreateStagingBuffer(
//                                        const UINT numElements,
//                                        const UINT sizeElement,
//                                        const void *initData,
//                                        ComPtr<ID3D11Buffer> &buffer) {
//
//     D3D11_BUFFER_DESC desc;
//     ZeroMemory(&desc, sizeof(desc));
//     desc.ByteWidth = numElements * sizeElement;
//     desc.Usage = D3D11_USAGE_STAGING;
//     desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
//     desc.StructureByteStride = sizeElement;
//
//     if (initData) {
//         D3D11_SUBRESOURCE_DATA bufferData;
//         ZeroMemory(&bufferData, sizeof(bufferData));
//         bufferData.pSysMem = initData;
//         ThrowIfFailed(GpuCore::Instance().device->CreateBuffer(
//             &desc, &bufferData, buffer.GetAddressOf()));
//     } else {
//         ThrowIfFailed(GpuCore::Instance().device->CreateBuffer(
//             &desc, NULL, buffer.GetAddressOf()));
//     }
// }
//
//  void Util::CopyToStagingBuffer(ComPtr<ID3D11Buffer> &buffer, UINT size,
//                                 void *src) {
//     D3D11_MAPPED_SUBRESOURCE ms;
//     GpuCore::Instance().device_context->Map(buffer.Get(), NULL,
//                                                  D3D11_MAP_WRITE, NULL, &ms);
//     memcpy(ms.pData, src, size);
//     GpuCore::Instance().device_context->Unmap(buffer.Get(), NULL);
// }

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
