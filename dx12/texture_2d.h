#ifndef _TEXTURE2D
#define _TEXTURE2D

#include "device_manager.h"
#include "gpu_resource.h"
#include "image.h"
#include <filesystem>
#include <logger.h>

namespace graphics {
class Texture2D : public GpuResource {
  public:
    Texture2D() : upload(0), cpu_handle_(), index_(0), is_usable_(false){};

    static Texture2D *Create(const std::string path1, const std::string path2) {

        auto image = Image::Read(path1, path2, false);
        Texture2D *tex = new Texture2D();

        if (!image.IsEmpty()) {
            tex->Initialize(image);
            tex->is_usable_ = true;
        } else { // dummy
            tex->Initialize(256, 256, DXGI_FORMAT_R8G8B8A8_UNORM);
            common::Logger::Debug(path1 + "," + path2 +
                                  ": not exists. skip texture reading.");
        }
        return tex;
    };
    static Texture2D *Create(const std::string path) {

        auto image = Image::Read(path, false);
        Texture2D *tex = new Texture2D();

        if (!image.IsEmpty()) {
            tex->Initialize(image);
            tex->is_usable_ = true;
        } else { // dummy
            tex->Initialize(256, 256, DXGI_FORMAT_R8G8B8A8_UNORM);
            common::Logger::Debug(path + ": not exists. skip texture reading.");
        }
        return tex;
    };
    static Texture2D *Create(const std::string metalic,
                             const std::string roughness,
                             bool is_metalic_roughness) {

        auto image = Image::ReadMetallicRoughness(metalic, roughness, false);
        Texture2D *tex = new Texture2D();

        if (!image.IsEmpty()) {
            tex->Initialize(image);
            tex->is_usable_ = true;
        } else { // dummy
            tex->Initialize(256, 256, DXGI_FORMAT_R8G8B8A8_UNORM);
            common::Logger::Debug(metalic + "," + roughness +
                                  ": not exists. skip texture reading.");
        }
        return tex;
    };
    static Texture2D *Create(int width, int height, DXGI_FORMAT format) {

        Texture2D *tex = new Texture2D();
        tex->Initialize(width, height, format);

        return tex;
    };
    D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() { return cpu_handle_; };
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() override {
        return GpuCore::Instance().GetHeap().View()->GetGpuHandle(index_);
    };
    bool IsUsable() { return is_usable_; };

  private:
    void Initialize(int width, int height, DXGI_FORMAT format) {
        auto context =
            GpuCore::Instance().GetCommand()->Begin<GraphicsCommandContext>(
                L"Texture2D");

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
        HRESULT hr = GpuCore::Instance().GetDevice()->CreateCommittedResource(
            &heapProperties, D3D12_HEAP_FLAG_NONE, &txtDesc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr,
            IID_PPV_ARGS(&resource_));

        GpuCore::Instance().GetCommand()->Finish(context, true);

        // view
        GpuCore::Instance().GetHeap().View()->AllocateDescriptor(cpu_handle_,
                                                                 index_);
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {
            resource_->GetDesc().Format, D3D12_SRV_DIMENSION_TEXTURE2D,
            D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING};

        desc.Texture2D.MipLevels = 1;

        GpuCore::Instance().GetDevice()->CreateShaderResourceView(
            resource_, &desc, cpu_handle_);
    };
    void Initialize(Image image) {
        current_state_ = D3D12_RESOURCE_STATE_COMMON;

        auto context =
            GpuCore::Instance().GetCommand()->Begin<GraphicsCommandContext>(
                L"Texture2D");

        D3D12_RESOURCE_DESC txtDesc;
        ZeroMemory(&txtDesc, sizeof(txtDesc));
        txtDesc.Width = image.Width();
        txtDesc.Height = image.Height();
        txtDesc.MipLevels = 0;
        txtDesc.DepthOrArraySize = 1;
        txtDesc.Format = image.Format();
        txtDesc.SampleDesc.Count = 1;
        txtDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        txtDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
        ASSERT_FAILED(GpuCore::Instance().GetDevice()->CreateCommittedResource(
            &heapProperties, D3D12_HEAP_FLAG_NONE, &txtDesc, current_state_,
            nullptr, IID_PPV_ARGS(&resource_)));

        // 2.upload heap
        const uint64_t uploadBufferSize =
            GetRequiredIntermediateSize(resource_, 0, 1);

        auto heap_property = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto buffer_size = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
        ASSERT_FAILED(GpuCore::Instance().GetDevice()->CreateCommittedResource(
            &heap_property, D3D12_HEAP_FLAG_NONE, &buffer_size, current_state_,
            nullptr, IID_PPV_ARGS(&upload)));

        // 3.copy data to upload heap
        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = image.Buffer().data();
        textureData.RowPitch = image.Width() * 4;
        textureData.SlicePitch = textureData.RowPitch * image.Height();

        context->TransitionResource(this, D3D12_RESOURCE_STATE_COPY_DEST, true);
        UpdateSubresources(context->GetList(), resource_, upload, 0, 0, 1,
                           &textureData);
        context->TransitionResource(
            this, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);

        GpuCore::Instance().GetCommand()->Finish(context, true);

        // view
        GpuCore::Instance().GetHeap().View()->AllocateDescriptor(cpu_handle_,
                                                                 index_);
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {
            resource_->GetDesc().Format, D3D12_SRV_DIMENSION_TEXTURE2D,
            D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING};

        desc.Texture2D.MipLevels = 1;

        GpuCore::Instance().GetDevice()->CreateShaderResourceView(
            resource_, &desc, cpu_handle_);
    };

    bool is_usable_;
    UINT index_;
    ID3D12Resource *upload;
    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle_;
};
} // namespace graphics
#endif
