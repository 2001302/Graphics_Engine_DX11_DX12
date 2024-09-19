#ifndef _TEXTURECUBE
#define _TEXTURECUBE

#include "dds_texture_loader.h"
#include "gpu_resource.h"
#include "graphics_util.h"
#include <filesystem>

namespace graphics {
class TextureCube : public GpuResource {
  public:
    TextureCube()
        : device_(0), heap_(0), resource_(0), cpu_handle_(), index_(0),
          file_name_(0), command_list_(0){};

    void Create(ID3D12Device *device, DynamicDescriptorHeap *heap,
                ID3D12GraphicsCommandList *command_list,
                const wchar_t *file_name) {
        device_ = device;
        heap_ = heap;
        file_name_ = file_name;
        command_list_ = command_list;
    };

    void Allocate() override {

        heap_->AllocateDescriptor(cpu_handle_, index_);

        CreateDDSTextureFromFile(device_, file_name_, 0, true, &resource_,
                                 cpu_handle_, result_);

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

        ASSERT_FAILED(device_->CreateCommittedResource(
            &heap_props, D3D12_HEAP_FLAG_NONE, &buffer_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&result_.UploadBuffer)));

        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            resource_, D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_COPY_DEST);
        command_list_->ResourceBarrier(1, &barrier);

        UpdateSubresources(command_list_, resource_, result_.UploadBuffer, 0, 0,
                           result_.NumSubresources, result_.initData.get());

        barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            resource_, D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        command_list_->ResourceBarrier(1, &barrier);
    };
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() override {
        return heap_->GetGpuHandle(index_);
    };
    ID3D12Resource *Get() { return resource_; };

  private:
    UINT index_;
    ID3D12Device *device_;
    DynamicDescriptorHeap *heap_;
    ID3D12Resource *resource_;
    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle_;
    TextureLoadResult result_;
    ID3D12GraphicsCommandList *command_list_;
    const wchar_t *file_name_;
};
} // namespace graphics
#endif
