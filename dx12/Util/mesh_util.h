#ifndef _GRAPHICSUTIL
#define _GRAPHICSUTIL

#include "../gpu/core/buffer_manager.h"
#include "../gpu/core/device_manager.h"
#include "../gpu/buffer/sampler_state.h"

namespace graphics {
class Util {
  public:
    static void CreateIndexBuffer(const std::vector<uint32_t> &indices,
                                  ComPtr<ID3D12Resource> &indexBuffer,
                                  D3D12_INDEX_BUFFER_VIEW &indexBufferView) {
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

        ASSERT_FAILED(GpuCore::Instance().GetDevice()->CreateCommittedResource(
            &heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&indexBuffer)));

        uint8_t *pIndexDataBegin;
        D3D12_RANGE readRange = {};
        ASSERT_FAILED(indexBuffer->Map(
            0, &readRange, reinterpret_cast<void **>(&pIndexDataBegin)));
        memcpy(pIndexDataBegin, indices.data(),
               indices.size() * sizeof(uint32_t));
        indexBuffer->Unmap(0, nullptr);

        indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
        indexBufferView.SizeInBytes =
            static_cast<UINT>(indices.size() * sizeof(uint32_t));
        indexBufferView.Format = DXGI_FORMAT_R32_UINT;
    }

    template <typename T_VERTEX>
    static void CreateVertexBuffer(const std::vector<T_VERTEX> &vertices,
                                   ComPtr<ID3D12Resource> &vertexBuffer,
                                   D3D12_VERTEX_BUFFER_VIEW &vertexBufferView) {
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

        ASSERT_FAILED(GpuCore::Instance().GetDevice()->CreateCommittedResource(
            &heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&vertexBuffer)));

        uint8_t *pVertexDataBegin;
        D3D12_RANGE readRange = {};
        ASSERT_FAILED(vertexBuffer->Map(
            0, &readRange, reinterpret_cast<void **>(&pVertexDataBegin)));
        memcpy(pVertexDataBegin, vertices.data(),
               vertices.size() * sizeof(T_VERTEX));
        vertexBuffer->Unmap(0, nullptr);

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

        ASSERT_FAILED(GpuCore::Instance().GetDevice()->CreateCommittedResource(
            &heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&constantBuffer)));

        UINT8 *begin = 0;
        D3D12_RANGE readRange = {};
        ASSERT_FAILED(constantBuffer->Map(0, &readRange,
                                          reinterpret_cast<void **>(&begin)));
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
        ASSERT_FAILED(
            buffer->Map(0, &readRange, reinterpret_cast<void **>(&begin)));
        memcpy(begin, bufferData.data(), sizeof(T_DATA) * bufferData.size());
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
        ASSERT_FAILED(
            buffer->Map(0, &readRange, reinterpret_cast<void **>(&begin)));
        memcpy(begin, &bufferData, sizeof(T_DATA));
        buffer->Unmap(0, nullptr);
    }

    template <typename T_STRUCTURED>
    static void CreateStructuredBuffer(const std::vector<T_STRUCTURED> &data,
                                       ComPtr<ID3D12Resource> &upload_buffer) {

        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Width = sizeof(T_STRUCTURED) * data.size();
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        // upload buffer : CPU access
        D3D12_HEAP_PROPERTIES props_upload = {};
        props_upload.Type = D3D12_HEAP_TYPE_UPLOAD;

        ASSERT_FAILED(GpuCore::Instance().GetDevice()->CreateCommittedResource(
            &props_upload, D3D12_HEAP_FLAG_NONE, &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&upload_buffer)));
    }
};
} // namespace graphics
#endif
