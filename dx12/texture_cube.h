#ifndef _TEXTURECUBE
#define _TEXTURECUBE

#include "dds_texture_loader.h"
#include "device_manager.h"
#include "gpu_resource.h"
#include <filesystem>

namespace graphics {
class TextureCube : public GpuResource {
  public:
    TextureCube() : cpu_handle_(), index_(0){};

    static TextureCube *Create(const wchar_t *file_name) {
        TextureCube *cube = new TextureCube();
        cube->Initialize(file_name);
        return cube;
    };

    D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() { return cpu_handle_; };
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() override {
        return GpuCore::Instance().GetHeap().View()->GetGpuHandle(index_);
    };

  private:
    void Initialize(const wchar_t *file_name) {
        current_state_ = D3D12_RESOURCE_STATE_COMMON;

        auto context =
            GpuCore::Instance().GetCommand()->Begin<GraphicsCommandContext>(
                L"TextureCube");

        GpuCore::Instance().GetHeap().View()->AllocateDescriptor(cpu_handle_,
                                                                 index_);

        CreateDDSTextureFromFile(GpuCore::Instance().GetDevice(), file_name, 0,
                                 true, &resource_, cpu_handle_, result_);

        UINT64 upload_buffer_size =
            GetRequiredIntermediateSize(resource_, 0, result_.NumSubresources);

        D3D12_HEAP_PROPERTIES heap_props;
        heap_props.Type = D3D12_HEAP_TYPE_UPLOAD;
        heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heap_props.CreationNodeMask = 1;
        heap_props.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC buffer_desc;
        buffer_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        buffer_desc.Alignment = 0;
        buffer_desc.Width = upload_buffer_size;
        buffer_desc.Height = 1;
        buffer_desc.DepthOrArraySize = 1;
        buffer_desc.MipLevels = 1;
        buffer_desc.Format = DXGI_FORMAT_UNKNOWN;
        buffer_desc.SampleDesc.Count = 1;
        buffer_desc.SampleDesc.Quality = 0;
        buffer_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        buffer_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        ASSERT_FAILED(GpuCore::Instance().GetDevice()->CreateCommittedResource(
            &heap_props, D3D12_HEAP_FLAG_NONE, &buffer_desc, current_state_,
            nullptr, IID_PPV_ARGS(&result_.UploadBuffer)));

        context->TransitionResource(this, D3D12_RESOURCE_STATE_COPY_DEST, true);
        UpdateSubresources(context->GetList(), resource_, result_.UploadBuffer,
                           0, 0, result_.NumSubresources,
                           result_.initData.get());
        context->TransitionResource(
            this, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);

        GpuCore::Instance().GetCommand()->Finish(context, true);
    };

    UINT index_;
    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle_;
    TextureLoadResult result_;
};
} // namespace graphics
#endif
