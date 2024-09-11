#ifndef _DEPTH_BUFFER
#define _DEPTH_BUFFER

#include "gpu_resource.h"

namespace graphics {
class DepthBuffer : public GpuResource {
  public:
    DepthBuffer()
        : device_(0), resource_(0), heap_DSV_(0), handle_DSV_(),
          use_MSAA_(false){};
    void Create(ID3D12Device *device, DescriptorHeap *heap_DSV,
                DXGI_FORMAT format, bool use_msaa = false) {
        device_ = device;
        heap_DSV_ = heap_DSV;

        D3D12_RESOURCE_DESC resource_desc_DSV = {};
        resource_desc_DSV.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resource_desc_DSV.Alignment = 0;
        resource_desc_DSV.Width = foundation::Env::Instance().screen_width;
        resource_desc_DSV.Height = foundation::Env::Instance().screen_height;
        resource_desc_DSV.DepthOrArraySize = 1;
        resource_desc_DSV.MipLevels = 1;
        resource_desc_DSV.Format = format;
        if (use_msaa) {
            resource_desc_DSV.SampleDesc.Count = 4;   // MSAA
            resource_desc_DSV.SampleDesc.Quality = 0; // MSAA
            use_MSAA_ = true;
        } else {
            resource_desc_DSV.SampleDesc.Count = 1;   // no MSAA
            resource_desc_DSV.SampleDesc.Quality = 0; // no MSAA
        }
        resource_desc_DSV.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resource_desc_DSV.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_CLEAR_VALUE clear_value_DSV = {};
        clear_value_DSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        clear_value_DSV.DepthStencil.Depth = 1.0f;
        clear_value_DSV.DepthStencil.Stencil = 0;

        auto heap_property = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        ASSERT_FAILED(device->CreateCommittedResource(
            &heap_property, D3D12_HEAP_FLAG_NONE, &resource_desc_DSV,
            D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear_value_DSV,
            IID_PPV_ARGS(&resource_)));
    };

    void Allocate() override {
        auto format = resource_->GetDesc().Format;

        auto dimension_RTV = use_MSAA_ ? D3D12_DSV_DIMENSION_TEXTURE2DMS
                                       : D3D12_DSV_DIMENSION_TEXTURE2D;
        UINT dsv_index = 0;
        heap_DSV_->AllocateDescriptor(handle_DSV_, dsv_index);

        D3D12_DEPTH_STENCIL_VIEW_DESC desc_DSV = {};
        desc_DSV.Format = format;
        desc_DSV.ViewDimension = dimension_RTV;
        desc_DSV.Flags = D3D12_DSV_FLAG_NONE;

        device_->CreateDepthStencilView(resource_, &desc_DSV, handle_DSV_);
    };
    D3D12_CPU_DESCRIPTOR_HANDLE GetDsvHandle() { return handle_DSV_; };

  private:
    ID3D12Device *device_;
    ID3D12Resource *resource_;
    DescriptorHeap *heap_DSV_;
    D3D12_CPU_DESCRIPTOR_HANDLE handle_DSV_;
    bool use_MSAA_ = false;
};
} // namespace graphics
#endif