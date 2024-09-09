#ifndef _COLOR_BUFFER
#define _COLOR_BUFFER

#include "gpu_resource.h"

namespace graphics {
class ColorBuffer {
  public:
    ColorBuffer()
        : heap_RTV_(0), heap_SRV_(0), handle_RTV_(), handle_SRV_(),
          use_MSAA_(false){};
    ColorBuffer(ID3D12Device *device, DXGI_FORMAT format, bool use_msaa = false)
        : ColorBuffer() {
        D3D12_RESOURCE_DESC resource_desc_RTV = {};
        resource_desc_RTV.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resource_desc_RTV.Alignment = 0;
        resource_desc_RTV.Width = foundation::Env::Instance().screen_width;
        resource_desc_RTV.Height = foundation::Env::Instance().screen_height;
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
        ThrowIfFailed(device->CreateCommittedResource(
            &heap_property, D3D12_HEAP_FLAG_NONE, &resource_desc_RTV,
            D3D12_RESOURCE_STATE_RENDER_TARGET, &clear_value_RTV,
            IID_PPV_ARGS(&resource_)));
    };

    void Allocate(ID3D12Device *device, DescriptorHeap *heap_RTV,
                  DescriptorHeap *heap_SRV) {
        auto format = resource_->GetDesc().Format;

        auto dimension_RTV = use_MSAA_ ? D3D12_RTV_DIMENSION_TEXTURE2DMS
                                       : D3D12_RTV_DIMENSION_TEXTURE2D;
        heap_RTV_ = heap_RTV;
        UINT rtv_index = 0;
        heap_RTV_->AllocateDescriptor(handle_RTV_, rtv_index);
        D3D12_RENDER_TARGET_VIEW_DESC desc_RTV = {format, dimension_RTV};
        device->CreateRenderTargetView(resource_.Get(), &desc_RTV, handle_RTV_);

        heap_SRV_ = heap_SRV;
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
        device->CreateShaderResourceView(resource_.Get(), &desc_SRV,
                                         handle_SRV_);
    };
    D3D12_CPU_DESCRIPTOR_HANDLE GetRtvHandle() { return handle_RTV_; };
    D3D12_CPU_DESCRIPTOR_HANDLE GetSrvHandle() { return handle_SRV_; };

  private:
    ComPtr<ID3D12Resource> resource_;
    DescriptorHeap *heap_RTV_;
    D3D12_CPU_DESCRIPTOR_HANDLE handle_RTV_;
    DescriptorHeap *heap_SRV_;
    D3D12_CPU_DESCRIPTOR_HANDLE handle_SRV_;
    const float clear_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    bool use_MSAA_ = false;
};
} // namespace graphics
#endif