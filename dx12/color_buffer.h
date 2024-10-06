#ifndef _COLOR_BUFFER
#define _COLOR_BUFFER

#include "device_manager.h"
#include "gpu_resource.h"

namespace graphics {
class ColorBuffer : public GpuResource {
  public:
    ColorBuffer()
        : handle_RTV_(), handle_SRV_(), use_MSAA_(false), rtv_index_(0),
          srv_index_(0){};
    static ColorBuffer *Create(int width, int height, DXGI_FORMAT format,
                               bool use_msaa = false) {
        auto color_buffer = new ColorBuffer();
        color_buffer->Initialize(width, height, format, use_msaa);
        return color_buffer;
    };

    D3D12_CPU_DESCRIPTOR_HANDLE GetRtvHandle() { return handle_RTV_; };
    D3D12_CPU_DESCRIPTOR_HANDLE GetSrvHandle() { return handle_SRV_; };
    D3D12_GPU_DESCRIPTOR_HANDLE GetRtvGpuHandle() {
        return GpuCore::Instance().GetHeap().RTV()->GetGpuHandle(rtv_index_);
    };
    D3D12_GPU_DESCRIPTOR_HANDLE GetSrvGpuHandle() {
        return GpuCore::Instance().GetHeap().View()->GetGpuHandle(srv_index_);
    };

  private:
    void Initialize(int width, int height, DXGI_FORMAT format,
                    bool use_msaa = false) {
        current_state_ = D3D12_RESOURCE_STATE_RENDER_TARGET;

        D3D12_RESOURCE_DESC resource_desc_RTV = {};
        resource_desc_RTV.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resource_desc_RTV.Alignment = 0;
        resource_desc_RTV.Width = width;
        resource_desc_RTV.Height = height;
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
        const float clear_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        memcpy(clear_value_RTV.Color, clear_color, sizeof(clear_color));

        auto heap_property = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        ASSERT_FAILED(GpuCore::Instance().GetDevice()->CreateCommittedResource(
            &heap_property, D3D12_HEAP_FLAG_NONE, &resource_desc_RTV,
            current_state_, &clear_value_RTV, IID_PPV_ARGS(&resource_)));

        auto dimension_RTV = use_MSAA_ ? D3D12_RTV_DIMENSION_TEXTURE2DMS
                                       : D3D12_RTV_DIMENSION_TEXTURE2D;

        GpuCore::Instance().GetHeap().RTV()->AllocateDescriptor(handle_RTV_,
                                                                rtv_index_);
        D3D12_RENDER_TARGET_VIEW_DESC desc_RTV = {format, dimension_RTV};
        GpuCore::Instance().GetDevice()->CreateRenderTargetView(
            resource_, &desc_RTV, handle_RTV_);

        GpuCore::Instance().GetHeap().View()->AllocateDescriptor(handle_SRV_,
                                                                 srv_index_);
        auto dimension_SRV = use_MSAA_ ? D3D12_SRV_DIMENSION_TEXTURE2DMS
                                       : D3D12_SRV_DIMENSION_TEXTURE2D;
        D3D12_SHADER_RESOURCE_VIEW_DESC desc_SRV = {
            format, dimension_SRV, D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING};
        desc_SRV.Texture2D.MostDetailedMip = 0;
        desc_SRV.Texture2D.MipLevels = 1;
        desc_SRV.Texture2D.PlaneSlice = 0;
        desc_SRV.Texture2D.ResourceMinLODClamp = 0.0f;
        GpuCore::Instance().GetDevice()->CreateShaderResourceView(
            resource_, &desc_SRV, handle_SRV_);
    };
    UINT rtv_index_;
    UINT srv_index_;
    D3D12_CPU_DESCRIPTOR_HANDLE handle_RTV_;
    D3D12_CPU_DESCRIPTOR_HANDLE handle_SRV_;
    bool use_MSAA_ = false;
};
} // namespace graphics
#endif