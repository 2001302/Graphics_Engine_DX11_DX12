#ifndef _GRAPHICSUTIL
#define _GRAPHICSUTIL

#include "graphics_core.h"
#include "graphics_pso.h"

namespace dx12 {
class Util {
  public:
    static void CreateIndexBuffer(const std::vector<uint32_t> &indices,
                                  ComPtr<ID3D12Resource> &indexBuffer,
                                  D3D12_INDEX_BUFFER_VIEW &indexBufferView) {
        // const UINT size = indices.size() * sizeof(uint32_t);
        // auto heap_property = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        // auto buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(size);

        // HRESULT hr = GpuCore::Instance().device->CreateCommittedResource(
        //     &heap_property, D3D12_HEAP_FLAG_NONE, &buffer_desc,
        //     D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        //     IID_PPV_ARGS(&indexBuffer));

        //// Initialize the vertex index view.
        // indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
        // indexBufferView.SizeInBytes = size;
        // indexBufferView.Format = DXGI_FORMAT_R32_UINT;

        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC resourceDesc = {};
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Width = indices.size() * sizeof(uint32_t);
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        dx12::ThrowIfFailed(
            dx12::GpuCore::Instance().device->CreateCommittedResource(
                &heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                IID_PPV_ARGS(&indexBuffer)));

        // 인덱스 데이터를 버퍼에 복사
        uint8_t *pIndexDataBegin;
        D3D12_RANGE readRange =
            {}; // We do not intend to read from this resource on the CPU.
        dx12::ThrowIfFailed(indexBuffer->Map(
            0, &readRange, reinterpret_cast<void **>(&pIndexDataBegin)));
        memcpy(pIndexDataBegin, indices.data(),
               indices.size() * sizeof(uint32_t));
        indexBuffer->Unmap(0, nullptr);

        // 인덱스 버퍼 뷰 설정
        indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
        indexBufferView.SizeInBytes =
            static_cast<UINT>(indices.size() * sizeof(uint32_t));
        indexBufferView.Format = DXGI_FORMAT_R32_UINT;
    }

    template <typename T_VERTEX>
    static void CreateVertexBuffer(const std::vector<T_VERTEX> &vertices,
                                   ComPtr<ID3D12Resource> &vertexBuffer,
                                   D3D12_VERTEX_BUFFER_VIEW &vertexBufferView) {
        // const UINT size = vertices.size() * sizeof(T_VERTEX);
        // auto heap_property = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        // auto buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(size);

        // HRESULT hr = GpuCore::Instance().device->CreateCommittedResource(
        //     &heap_property, D3D12_HEAP_FLAG_NONE, &buffer_desc,
        //     D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        //     IID_PPV_ARGS(&vertexBuffer));

        //// Initialize the vertex buffer view.
        // vertexBufferView.BufferLocation =
        // vertexBuffer->GetGPUVirtualAddress(); vertexBufferView.StrideInBytes
        // = sizeof(T_VERTEX); vertexBufferView.SizeInBytes = size;

        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC resourceDesc = {};
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Width = vertices.size() * sizeof(T_VERTEX);
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        dx12::ThrowIfFailed(
            dx12::GpuCore::Instance().device->CreateCommittedResource(
                &heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                IID_PPV_ARGS(&vertexBuffer)));

        // Vertex 데이터를 버퍼에 복사
        uint8_t *pVertexDataBegin;
        D3D12_RANGE readRange =
            {}; // We do not intend to read from this resource on the CPU.
        dx12::ThrowIfFailed(vertexBuffer->Map(
            0, &readRange, reinterpret_cast<void **>(&pVertexDataBegin)));
        memcpy(pVertexDataBegin, vertices.data(),
               vertices.size() * sizeof(T_VERTEX));
        vertexBuffer->Unmap(0, nullptr);

        // Vertex 버퍼 뷰 설정
        vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
        vertexBufferView.SizeInBytes =
            static_cast<UINT>(vertices.size() * sizeof(T_VERTEX));
        vertexBufferView.StrideInBytes = sizeof(T_VERTEX);
    }

    template <typename T_CONSTANT>
    static void CreateConstBuffer(const T_CONSTANT &constantBufferData,
                                  ComPtr<ID3D12Resource> &constantBuffer) {

        static_assert((sizeof(T_CONSTANT) % 16) == 0,
                      "Constant Buffer size must be 16-byte aligned");

        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC resourceDesc = {};
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Width = sizeof(T_CONSTANT);
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        dx12::ThrowIfFailed(
            dx12::GpuCore::Instance().device->CreateCommittedResource(
                &heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                IID_PPV_ARGS(&constantBuffer)));

        UINT8 *begin = 0;
        D3D12_RANGE readRange = {};
        dx12::ThrowIfFailed(constantBuffer->Map(
            0, &readRange, reinterpret_cast<void **>(&begin)));
        memcpy(begin, &constantBufferData, sizeof(constantBufferData));
        constantBuffer->Unmap(0, nullptr);
    }

    template <typename T_DATA>
    static void UpdateBuffer(const std::vector<T_DATA> &bufferData,
                             ComPtr<ID3D12Resource> &buffer) {

        if (!buffer) {
            std::cout << "UpdateBuffer() buffer was not initialized."
                      << std::endl;
        }

        UINT8 *begin = 0;
        CD3DX12_RANGE readRange(0, 0);
        ThrowIfFailed(
            buffer->Map(0, &readRange, reinterpret_cast<void **>(&begin)));
        memcpy(begin, &bufferData.data(), sizeof(T_DATA) * bufferData.size());
        buffer->Unmap(0, nullptr);
    }

    template <typename T_DATA>
    static void UpdateBuffer(const T_DATA &bufferData,
                             ComPtr<ID3D12Resource> &buffer) {

        if (!buffer) {
            std::cout << "UpdateBuffer() buffer was not initialized."
                      << std::endl;
        }

        UINT8 *begin = 0;
        CD3DX12_RANGE readRange(0, 0);
        ThrowIfFailed(
            buffer->Map(0, &readRange, reinterpret_cast<void **>(&begin)));
        memcpy(begin, &bufferData, sizeof(T_DATA));
        buffer->Unmap(0, nullptr);
    }

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
    static void
    CreateVertexShader(ComPtr<ID3D12Device> &device, std::wstring filename,
                       ComPtr<ID3DBlob> &m_vertexShader,
                       const std::vector<D3D_SHADER_MACRO> shaderMacros = {
                           /* Empty default */});

    static void CreatePixelShader(ComPtr<ID3D12Device> &device,
                                  std::wstring filename,
                                  ComPtr<ID3DBlob> &shader);
};
} // namespace dx12
#endif
