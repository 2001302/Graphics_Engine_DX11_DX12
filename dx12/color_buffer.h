#ifndef _COLOR_BUFFER
#define _COLOR_BUFFER

#include "gpu_resource.h"

namespace graphics {
class ColorBuffer : public GpuResource {
  public:
    ColorBuffer()
        : heap_RTV_(0), heap_SRV_(0), handle_RTV_(), handle_SRV_(),
          use_MSAA_(false), rtv_index_(0), srv_index_(0){};
    void Create(ID3D12Device *device, DynamicDescriptorHeap *heap_RTV,
                DynamicDescriptorHeap *heap_SRV, DXGI_FORMAT format,
                bool use_msaa = false) {
        heap_RTV_ = heap_RTV;
        heap_SRV_ = heap_SRV;
        current_state_ = D3D12_RESOURCE_STATE_RENDER_TARGET;

        D3D12_RESOURCE_DESC resource_desc_RTV = {};
        resource_desc_RTV.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resource_desc_RTV.Alignment = 0;
        resource_desc_RTV.Width = common::env::screen_width;
        resource_desc_RTV.Height = common::env::screen_height;
        resource_desc_RTV.DepthOrArraySize = 1;
        resource_desc_RTV.MipLevels = 1;
        resource_desc_RTV.Format = format;
        if (use_msaa) {
            resource_desc_RTV.SampleDesc.Count = 4;   // MSAA
            resource_desc_RTV.SampleDesc.Quality = 0; // MSAA
            use_MSAA_ = true;
        } else {
            resource_desc_RTV.SampleDesc.Count = 1;   // no MSAA
            resource_desc_RTV.SampleDesc.Quality = 0; // no MSAA
        }
        resource_desc_RTV.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resource_desc_RTV.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        D3D12_CLEAR_VALUE clear_value_RTV = {};
        clear_value_RTV.Format = format;
        memcpy(clear_value_RTV.Color, clear_color, sizeof(clear_color));

        auto heap_property = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        ASSERT_FAILED(device->CreateCommittedResource(
            &heap_property, D3D12_HEAP_FLAG_NONE, &resource_desc_RTV,
            current_state_, &clear_value_RTV, IID_PPV_ARGS(&resource_)));

        auto dimension_RTV = use_MSAA_ ? D3D12_RTV_DIMENSION_TEXTURE2DMS
                                       : D3D12_RTV_DIMENSION_TEXTURE2D;

        heap_RTV_->AllocateDescriptor(handle_RTV_, rtv_index_);
        D3D12_RENDER_TARGET_VIEW_DESC desc_RTV = {format, dimension_RTV};
        device->CreateRenderTargetView(resource_, &desc_RTV, handle_RTV_);

        heap_SRV_->AllocateDescriptor(handle_SRV_, srv_index_);
        auto dimension_SRV = use_MSAA_ ? D3D12_SRV_DIMENSION_TEXTURE2DMS
                                       : D3D12_SRV_DIMENSION_TEXTURE2D;
        D3D12_SHADER_RESOURCE_VIEW_DESC desc_SRV = {
            format, dimension_SRV, D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING};
        desc_SRV.Texture2D.MostDetailedMip = 0;
        desc_SRV.Texture2D.MipLevels = 1;
        desc_SRV.Texture2D.PlaneSlice = 0;
        desc_SRV.Texture2D.ResourceMinLODClamp = 0.0f;
        device->CreateShaderResourceView(resource_, &desc_SRV, handle_SRV_);
    };

    D3D12_CPU_DESCRIPTOR_HANDLE GetRtvHandle() { return handle_RTV_; };
    D3D12_CPU_DESCRIPTOR_HANDLE GetSrvHandle() { return handle_SRV_; };
    D3D12_GPU_DESCRIPTOR_HANDLE GetRtvGpuHandle() {
        return heap_RTV_->GetGpuHandle(rtv_index_);
    };
    D3D12_GPU_DESCRIPTOR_HANDLE GetSrvGpuHandle() {
        return heap_SRV_->GetGpuHandle(srv_index_);
    };
    const float *ClearColor() { return clear_color; };

  private:
    UINT rtv_index_;
    UINT srv_index_;
    DynamicDescriptorHeap *heap_RTV_;
    DynamicDescriptorHeap *heap_SRV_;
    D3D12_CPU_DESCRIPTOR_HANDLE handle_RTV_;
    D3D12_CPU_DESCRIPTOR_HANDLE handle_SRV_;
    const float clear_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    bool use_MSAA_ = false;
};
} // namespace graphics
#endif