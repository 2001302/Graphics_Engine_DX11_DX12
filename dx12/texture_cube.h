#ifndef _TEXTURECUBE
#define _TEXTURECUBE

#include "dds_texture_loader.h"
#include "device_manager.h"
#include "gpu_resource.h"
#include <filesystem>

namespace graphics {
class TextureCube : public GpuResource {
  public:
    TextureCube() : cpu_handle_(), index_(0), is_black_(false){};

    static TextureCube *Create(const wchar_t *file_name, bool is_brdf = false) {
        TextureCube *cube = new TextureCube();

        if (file_name != nullptr && std::filesystem::exists(file_name)) {
            cube->Initialize(file_name);
        } else {
            cube->Initialize(256, 256, is_brdf);
            cube->is_black_ = true;
        }
        return cube;
    };

    D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() { return cpu_handle_; };
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() override {
        return GpuCore::Instance().GetHeap().View()->GetGpuHandle(index_);
    };
    bool IsBlack() { return is_black_; };

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
    void Initialize(int width, int height, bool is_brdf) {
        auto context =
            GpuCore::Instance().GetCommand()->Begin<GraphicsCommandContext>(
                L"TextureCube");

        D3D12_RESOURCE_DESC txtDesc;
        ZeroMemory(&txtDesc, sizeof(txtDesc));
        txtDesc.Width = width;
        txtDesc.Height = height;
        txtDesc.MipLevels = 0;
        txtDesc.DepthOrArraySize = 6; // 6 faces for a cube map
        txtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

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
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        desc.Format =
            DXGI_FORMAT_R8G8B8A8_UNORM; // Use a compatible format for SRV
        desc.ViewDimension =
            is_brdf ? D3D12_SRV_DIMENSION_TEXTURE2D
                    : D3D12_SRV_DIMENSION_TEXTURECUBE; 
        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        desc.TextureCube.MipLevels = 1;

        GpuCore::Instance().GetDevice()->CreateShaderResourceView(
            resource_, &desc, cpu_handle_);
    }

    UINT index_;
    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle_;
    TextureLoadResult result_;
    bool is_black_;
};
} // namespace graphics
#endif
