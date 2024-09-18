#ifndef _COLOR_BUFFER
#define _COLOR_BUFFER

#include "gpu_resource.h"

namespace graphics {
class ColorBuffer : GpuResource {
  public:
    ColorBuffer()
        : device_(0), resource_(0), heap_RTV_(0), heap_SRV_(0), handle_RTV_(),
          handle_SRV_(), use_MSAA_(false), current_state_(){};
    void Create(ID3D12Device *device, DynamicDescriptorHeap *heap_RTV,
                DynamicDescriptorHeap *heap_SRV, DXGI_FORMAT format,
                bool use_msaa = false) {
        heap_RTV_ = heap_RTV;
        heap_SRV_ = heap_SRV;
        device_ = device;
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
    };

    void Allocate() override {
        auto format = resource_->GetDesc().Format;

        auto dimension_RTV = use_MSAA_ ? D3D12_RTV_DIMENSION_TEXTURE2DMS
                                       : D3D12_RTV_DIMENSION_TEXTURE2D;
        UINT rtv_index = 0;
        heap_RTV_->AllocateDescriptor(handle_RTV_, rtv_index);
        D3D12_RENDER_TARGET_VIEW_DESC desc_RTV = {format, dimension_RTV};
        device_->CreateRenderTargetView(resource_, &desc_RTV, handle_RTV_);

        UINT srv_index = 0;
        heap_SRV_->AllocateDescriptor(handle_SRV_, srv_index);
        auto dimension_SRV = use_MSAA_ ? D3D12_SRV_DIMENSION_TEXTURE2DMS
                                       : D3D12_SRV_DIMENSION_TEXTURE2D;
        D3D12_SHADER_RESOURCE_VIEW_DESC desc_SRV = {
            format, dimension_SRV, D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING};
        desc_SRV.Texture2D.MostDetailedMip = 0;
        desc_SRV.Texture2D.MipLevels = 1;
        desc_SRV.Texture2D.PlaneSlice = 0;
        desc_SRV.Texture2D.ResourceMinLODClamp = 0.0f;
        device_->CreateShaderResourceView(resource_, &desc_SRV, handle_SRV_);
    };
    ID3D12Resource *Get() { return resource_; };
    D3D12_CPU_DESCRIPTOR_HANDLE GetRtvHandle() { return handle_RTV_; };
    D3D12_CPU_DESCRIPTOR_HANDLE GetSrvHandle() { return handle_SRV_; };
    D3D12_RESOURCE_STATES GetCurrentState() { return current_state_; };
    void SetCurrentState(D3D12_RESOURCE_STATES state) {
        current_state_ = state;
    };
    const float *ClearColor() { return clear_color; };


  private:
    ID3D12Device *device_;
    ID3D12Resource *resource_;
    DynamicDescriptorHeap *heap_RTV_;
    DynamicDescriptorHeap *heap_SRV_;
    D3D12_RESOURCE_STATES current_state_;
    D3D12_CPU_DESCRIPTOR_HANDLE handle_RTV_;
    D3D12_CPU_DESCRIPTOR_HANDLE handle_SRV_;
    const float clear_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    bool use_MSAA_ = false;
};
} // namespace graphics
#endif