#ifndef _TEXTURE2D
#define _TEXTURE2D

#include "gpu_buffer.h"
#include <filesystem>

namespace dx12 {
class Texture2D : public GpuBuffer {
  public:
    Texture2D() : GpuBuffer(){};
    Texture2D(int width, int height, DXGI_FORMAT format) {

        D3D12_RESOURCE_DESC txtDesc;
        ZeroMemory(&txtDesc, sizeof(txtDesc));
        txtDesc.Width = width;
        txtDesc.Height = height;
        txtDesc.MipLevels = 0;
        txtDesc.DepthOrArraySize = 1;
        txtDesc.Format = format;
        txtDesc.SampleDesc.Count = 1;
        txtDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        txtDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
        HRESULT hr = dx12::GpuCore::Instance().device->CreateCommittedResource(
            &heapProperties, D3D12_HEAP_FLAG_NONE, &txtDesc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr,
            IID_PPV_ARGS(buffer_.GetAddressOf()));
    }
    void Allocate(GpuHeap *heap, UINT &index) override {
        GpuBuffer::Allocate(heap, index);

        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {
            buffer_->GetDesc().Format, D3D12_SRV_DIMENSION_TEXTURE2D,
            D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING};

        desc.Texture2D.MipLevels = buffer_->GetDesc().MipLevels;

        GpuCore::Instance().device->CreateShaderResourceView(
            buffer_.Get(), &desc, cpu_handle_);
    }
};
} // namespace dx12
#endif
