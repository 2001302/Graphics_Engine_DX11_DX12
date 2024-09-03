#include "texture_cube.h"

namespace dx12 {
TextureCube::TextureCube(const wchar_t *file_name,
                         ComPtr<ID3D12GraphicsCommandList> command_list,
                         bool isCubeMap, bool isBrdf) {

    DirectX::ResourceUploadBatch upload(dx12::GpuCore::Instance().device.Get());
    upload.Begin();
    DirectX::CreateDDSTextureFromFile(dx12::GpuCore::Instance().device.Get(),
                                      upload, file_name, &buffer_, false,
                                      isCubeMap);
    auto finished = upload.End(dx12::GpuCore::Instance().command_queue.Get());
    finished.wait();

    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        buffer_.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    command_list->ResourceBarrier(1, &barrier);

    isBrdf_ = isBrdf;
}

void TextureCube::Allocate(GpuHeap *heap, UINT &index) {
    GpuBuffer::Allocate(heap, index);

    if (!isBrdf_) {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {
            buffer_->GetDesc().Format, D3D12_SRV_DIMENSION_TEXTURECUBE,
            D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING};

        desc.TextureCube.MipLevels = buffer_->GetDesc().MipLevels;
        desc.TextureCube.MostDetailedMip = 0;
        desc.TextureCube.ResourceMinLODClamp = 0;

        GpuCore::Instance().device->CreateShaderResourceView(
            buffer_.Get(), &desc, cpu_handle_);
    } else {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {
            buffer_->GetDesc().Format, D3D12_SRV_DIMENSION_TEXTURE2D,
            D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING};

        desc.Texture2D.MipLevels = buffer_->GetDesc().MipLevels;

        GpuCore::Instance().device->CreateShaderResourceView(
            buffer_.Get(), &desc, cpu_handle_);
    }
}
} // namespace dx12