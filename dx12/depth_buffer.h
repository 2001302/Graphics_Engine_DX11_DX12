#ifndef _DEPTH_BUFFER
#define _DEPTH_BUFFER

#include "gpu_resource.h"
#include "device_manager.h"

namespace graphics {
class DepthBuffer : public GpuResource {
  public:
    DepthBuffer() : handle_DSV_(), handle_SRV_(){};
    static DepthBuffer *Create(int width, int height, DXGI_FORMAT format,
								bool use_msaa = false) {
		auto depth_buffer = new DepthBuffer();
        depth_buffer->Initialize(width, height, format, use_msaa);
		return depth_buffer;
    };
    D3D12_CPU_DESCRIPTOR_HANDLE GetDsvHandle() { return handle_DSV_; };
    D3D12_CPU_DESCRIPTOR_HANDLE GetSrvHandle() { return handle_SRV_; };

  private:
    void Initialize(int width,int height, DXGI_FORMAT format, bool use_msaa = false) {
        current_state_ = D3D12_RESOURCE_STATE_DEPTH_WRITE;
        D3D12_RESOURCE_DESC resource_desc_DSV = {};
        resource_desc_DSV.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resource_desc_DSV.Alignment = 0;
        resource_desc_DSV.Width = width;
        resource_desc_DSV.Height = height;
        resource_desc_DSV.DepthOrArraySize = 1;
        resource_desc_DSV.MipLevels = 1;
        resource_desc_DSV.Format = DXGI_FORMAT_R24G8_TYPELESS;
        if (use_msaa) {
            resource_desc_DSV.SampleDesc.Count = 4;   // MSAA
            resource_desc_DSV.SampleDesc.Quality = 0; // MSAA
        } else {
            resource_desc_DSV.SampleDesc.Count = 1;   // no MSAA
            resource_desc_DSV.SampleDesc.Quality = 0; // no MSAA
        }
        resource_desc_DSV.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resource_desc_DSV.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_CLEAR_VALUE clear_value_DSV = {};
        clear_value_DSV.Format = format;
        clear_value_DSV.DepthStencil.Depth = 1.0f;
        clear_value_DSV.DepthStencil.Stencil = 0;

        auto heap_property = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        ASSERT_FAILED(GpuCore::Instance().GetDevice()->CreateCommittedResource(
            &heap_property, D3D12_HEAP_FLAG_NONE, &resource_desc_DSV,
            current_state_, &clear_value_DSV, IID_PPV_ARGS(&resource_)));

        // depth stencil view
        auto dimension_DSV = use_msaa ? D3D12_DSV_DIMENSION_TEXTURE2DMS
                                       : D3D12_DSV_DIMENSION_TEXTURE2D;
        UINT dsv_index = 0;
        GpuCore::Instance().GetHeap().DSV()->AllocateDescriptor(handle_DSV_,
                                                          dsv_index);

        D3D12_DEPTH_STENCIL_VIEW_DESC desc_DSV = {};
        desc_DSV.Format = format;
        desc_DSV.ViewDimension = dimension_DSV;
        desc_DSV.Flags = D3D12_DSV_FLAG_NONE;

        GpuCore::Instance().GetDevice()->CreateDepthStencilView(resource_, &desc_DSV, handle_DSV_);

        //shader resource view
        auto dimension_SRV = use_msaa ? D3D12_SRV_DIMENSION_TEXTURE2DMS
                                      : D3D12_SRV_DIMENSION_TEXTURE2D;

        UINT srv_index = 0;
        GpuCore::Instance().GetHeap().View()->AllocateDescriptor(handle_SRV_,
                                                                 srv_index);
        D3D12_SHADER_RESOURCE_VIEW_DESC desc_SRV = {};
        desc_SRV.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        desc_SRV.ViewDimension = dimension_SRV;
        desc_SRV.Shader4ComponentMapping =
            D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        desc_SRV.Texture2D.MipLevels = 1;

        GpuCore::Instance().GetDevice()->CreateShaderResourceView(
            resource_, &desc_SRV, handle_SRV_);
    };

    D3D12_CPU_DESCRIPTOR_HANDLE handle_DSV_;
    D3D12_CPU_DESCRIPTOR_HANDLE handle_SRV_;
};
} // namespace graphics
#endif