#ifndef _TEXTURECUBE
#define _TEXTURECUBE

#include "gpu_resource.h"
#include <directxtk12/DDSTextureLoader.h>
#include <directxtk12/ResourceUploadBatch.h>
#include <filesystem>

namespace graphics {
class TextureCube : public GpuResource {
  public:
    TextureCube()
        : device_(0), heap_(0), resource_(0), cpu_handle_(), index_(0),
          isBrdf_(0){};

    void Create(ID3D12Device *device, DescriptorHeap *heap,
                const wchar_t *file_name,
                ComPtr<ID3D12GraphicsCommandList> command_list, bool isCubeMap,
                bool isBrdf = false) {
        device_ = device;
        DirectX::ResourceUploadBatch upload(device_);
        upload.Begin();
        DirectX::CreateDDSTextureFromFile(device_, upload, file_name,
                                          &resource_, false, isCubeMap);
        //auto finished = upload.End(GpuCore::Instance()
        //                               .GetCommandMgr()
        //                               ->GetCopyQueue()
        //                               .GetCommandQueue());
        //finished.wait();

        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            resource_, D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        command_list->ResourceBarrier(1, &barrier);

        isBrdf_ = isBrdf;
        heap_ = heap;
    };

    void Allocate() override {

        heap_->AllocateDescriptor(cpu_handle_, index_);

        if (!isBrdf_) {
            D3D12_SHADER_RESOURCE_VIEW_DESC desc = {
                resource_->GetDesc().Format, D3D12_SRV_DIMENSION_TEXTURECUBE,
                D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING};

            desc.TextureCube.MipLevels = resource_->GetDesc().MipLevels;
            desc.TextureCube.MostDetailedMip = 0;
            desc.TextureCube.ResourceMinLODClamp = 0;

            device_->CreateShaderResourceView(resource_, &desc, cpu_handle_);
        } else {
            D3D12_SHADER_RESOURCE_VIEW_DESC desc = {
                resource_->GetDesc().Format, D3D12_SRV_DIMENSION_TEXTURE2D,
                D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING};

            desc.Texture2D.MipLevels = resource_->GetDesc().MipLevels;

            device_->CreateShaderResourceView(resource_, &desc, cpu_handle_);
        }
    };

    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() override {
        return heap_->GetGpuHandle(index_);
    };

  private:
    bool isBrdf_;
    UINT index_;
    ID3D12Device *device_;
    DescriptorHeap *heap_;
    ID3D12Resource *resource_;
    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle_;
};
} // namespace graphics
#endif