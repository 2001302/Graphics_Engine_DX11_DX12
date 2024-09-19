#ifndef _TEXTURE2D
#define _TEXTURE2D

#include "gpu_resource.h"
#include "image.h"
#include <filesystem>

namespace graphics {
class Texture2D : public GpuResource {
  public:
    Texture2D()
        : device_(0), heap_(0), upload(0), resource_(0), cpu_handle_(),
          index_(0){};
    void Create(ID3D12Device *device, DynamicDescriptorHeap *heap, int width,
                int height, DXGI_FORMAT format) {
        device_ = device;
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
        HRESULT hr = device_->CreateCommittedResource(
            &heapProperties, D3D12_HEAP_FLAG_NONE, &txtDesc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr,
            IID_PPV_ARGS(&resource_));

        heap_ = heap;
    };
    void Create(ID3D12Device *device, DynamicDescriptorHeap *heap, Image image,
                ComPtr<ID3D12GraphicsCommandList> command_list) {
        device_ = device;
        heap_ = heap;
        CreateTextureHelper(&image, command_list.Get());
    };
    void Allocate() override {
        heap_->AllocateDescriptor(cpu_handle_, index_);

        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {
            resource_->GetDesc().Format, D3D12_SRV_DIMENSION_TEXTURE2D,
            D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING};

        desc.Texture2D.MipLevels = 1;

        device_->CreateShaderResourceView(resource_, &desc, cpu_handle_);
    };

  private:
    void CreateTextureHelper(
        Image *image,
        ID3D12GraphicsCommandList *command_list) { // 1.resource creation
        D3D12_RESOURCE_DESC txtDesc;
        ZeroMemory(&txtDesc, sizeof(txtDesc));
        txtDesc.Width = image->Width();
        txtDesc.Height = image->Height();
        txtDesc.MipLevels = 0;
        txtDesc.DepthOrArraySize = 1;
        txtDesc.Format = image->Format();
        txtDesc.SampleDesc.Count = 1;
        txtDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        txtDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
        ASSERT_FAILED(device_->CreateCommittedResource(
            &heapProperties, D3D12_HEAP_FLAG_NONE, &txtDesc,
            D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&resource_)));

        // 2.upload heap
        const uint64_t uploadBufferSize =
            GetRequiredIntermediateSize(resource_, 0, 1);

        auto heap_property = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto buffer_size = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
        ASSERT_FAILED(device_->CreateCommittedResource(
            &heap_property, D3D12_HEAP_FLAG_NONE, &buffer_size,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload)));

        // 3.copy data to upload heap
        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = image->Buffer().data();
        textureData.RowPitch = image->Width() * 4;
        textureData.SlicePitch = textureData.RowPitch * image->Height();

        UpdateSubresources(command_list, resource_, upload, 0, 0, 1,
                           &textureData);

        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            resource_, D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        command_list->ResourceBarrier(1, &barrier);
    };

    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() override {
        return heap_->GetGpuHandle(index_);
    };

    UINT index_;
    ID3D12Device *device_;
    DynamicDescriptorHeap *heap_;
    ID3D12Resource *upload;
    ID3D12Resource *resource_;
    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle_;
};
} // namespace graphics
#endif
