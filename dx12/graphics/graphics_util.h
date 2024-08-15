#ifndef _GRAPHICSUTIL
#define _GRAPHICSUTIL

#include "graphics_core.h"

namespace dx12 {
class Util {
  public:
    static void CreateIndexBuffer(const std::vector<uint32_t> &indices,
                                  ComPtr<ID3D12Resource> &indexBuffer) {
        const UINT size = sizeof(indices);
        auto heap_desc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(size);

        HRESULT hr = GpuCore::Instance().device->CreateCommittedResource(
            &heap_desc, D3D12_HEAP_FLAG_NONE, &buffer_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&indexBuffer));
    }

    template <typename T_VERTEX>
    static void CreateVertexBuffer(const std::vector<T_VERTEX> &vertices,
                                   ComPtr<ID3D12Resource> &vertexBuffer,
                                   D3D12_VERTEX_BUFFER_VIEW &vertexBufferView) {

        const UINT size = sizeof(vertices);

        HRESULT hr = GpuCore::Instance().device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(size),
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&vertexBuffer));

        // Initialize the vertex buffer view.
        vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
        vertexBufferView.StrideInBytes = sizeof(T_VERTEX);
        vertexBufferView.SizeInBytes = size;
    }

    template <typename T_CONSTANT>
    static void CreateConstBuffer(const T_CONSTANT &constantBufferData,
                                  ComPtr<ID3D12Resource> &constantBuffer) {

        static_assert((sizeof(T_CONSTANT) % 16) == 0,
                      "Constant Buffer size must be 16-byte aligned");

        const UINT size = sizeof(constantBufferData);

        HRESULT hr = GpuCore::Instance().device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(size),
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&constantBuffer));

        D3D12_RANGE readRange = {};
        ThrowIfFailed(constantBuffer->Map(
            0, &readRange, reinterpret_cast<void **>(&constantBufferData)));
    }

    // template <typename T_DATA>
    // static void UpdateBuffer(const std::vector<T_DATA> &bufferData,
    //                          ComPtr<ID3D12Resource> &buffer) {

    //    if (!buffer) {
    //        std::cout << "UpdateBuffer() buffer was not initialized."
    //                  << std::endl;
    //    }

    //    D3D11_MAPPED_SUBRESOURCE ms;
    //    GpuCore::Instance().device_context->Map(
    //        buffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
    //    memcpy(ms.pData, bufferData.data(), sizeof(T_DATA) *
    //    bufferData.size());
    //    GpuCore::Instance().device_context->Unmap(buffer.Get(), NULL);
    //}

    // template <typename T_DATA>
    // static void UpdateBuffer(const T_DATA &bufferData,
    //                          ComPtr<ID3D12Resource> &buffer) {

    //    if (!buffer) {
    //        std::cout << "UpdateBuffer() buffer was not initialized."
    //                  << std::endl;
    //    }

    //    D3D11_MAPPED_SUBRESOURCE ms;
    //    GpuCore::Instance().device_context->Map(
    //        buffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
    //    memcpy(ms.pData, &bufferData, sizeof(bufferData));
    //    GpuCore::Instance().device_context->Unmap(buffer.Get(), NULL);
    //}

    // static void CreateTexture(

    //    const std::string filename, const bool usSRGB,
    //    ComPtr<ID3D11Texture2D> &texture,
    //    ComPtr<ID3D11ShaderResourceView> &textureResourceView);

    // static void CreateTexture(

    //    const std::string albedoFilename, const std::string opacityFilename,
    //    const bool usSRGB, ComPtr<ID3D11Texture2D> &texture,
    //    ComPtr<ID3D11ShaderResourceView> &textureResourceView);

    // static void CreateMetallicRoughnessTexture(

    //    const std::string metallicFiilename,
    //    const std::string roughnessFilename, ComPtr<ID3D11Texture2D> &texture,
    //    ComPtr<ID3D11ShaderResourceView> &srv);

    // static void CreateTextureArray(

    //    const std::vector<std::string> filenames,
    //    ComPtr<ID3D11Texture2D> &texture,
    //    ComPtr<ID3D11ShaderResourceView> &textureResourceView);

    // static void CreateDDSTexture(const wchar_t *filename, const bool
    // isCubeMap,
    //                              ComPtr<ID3D11ShaderResourceView>
    //                              &texResView);

    // static void WriteToFile(

    //    ComPtr<ID3D11Texture2D> &textureToWrite, const std::string filename);

    // static void CreateUATexture(const int width, const int height,
    //                             const DXGI_FORMAT pixelFormat,
    //                             ComPtr<ID3D11Texture2D> &texture,
    //                             ComPtr<ID3D11RenderTargetView> &rtv,
    //                             ComPtr<ID3D11ShaderResourceView> &srv,
    //                             ComPtr<ID3D11UnorderedAccessView> &uav);

    // static void CreateTexture3D(const int width, const int height,
    //                             const int depth, const DXGI_FORMAT
    //                             pixelFormat, const std::vector<float>
    //                             &initData, ComPtr<ID3D11Texture3D> &texture,
    //                             ComPtr<ID3D11RenderTargetView> &rtv,
    //                             ComPtr<ID3D11ShaderResourceView> &srv,
    //                             ComPtr<ID3D11UnorderedAccessView> &uav);

    // static ComPtr<ID3D11Texture3D>
    // CreateStagingTexture3D(const int width, const int height, const int
    // depth,
    //                        const DXGI_FORMAT pixelFormat);

    // static size_t GetPixelSize(DXGI_FORMAT pixelFormat);

    // static void ComputeShaderBarrier();

    // static void CreateStructuredBuffer(const UINT numElements,
    //                                    const UINT sizeElement,
    //                                    const void *initData,
    //                                    ComPtr<ID3D11Buffer> &buffer,
    //                                    ComPtr<ID3D11ShaderResourceView> &srv,
    //                                    ComPtr<ID3D11UnorderedAccessView>
    //                                    &uav);

    // static void CreateStagingBuffer(const UINT numElements,
    //                                 const UINT sizeElement,
    //                                 const void *initData,
    //                                 ComPtr<ID3D11Buffer> &buffer);

    // static void CopyToStagingBuffer(ComPtr<ID3D11Buffer> &buffer, UINT size,
    //                                 void *src);

    // static void SetPipelineState(const PSO &pso);

    // static void SetGlobalConsts(ComPtr<ID3D11Buffer> &globalConstsGPU);
};
} // namespace dx12
#endif