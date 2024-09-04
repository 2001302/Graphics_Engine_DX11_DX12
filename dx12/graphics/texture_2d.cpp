#include "texture_2d.h"

namespace graphics {
void Texture2D::CreateTextureHelper(
    Image *image, ComPtr<ID3D12Resource> &texture,
    ComPtr<ID3D12GraphicsCommandList> command_list) {
    // 1.resource creation
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
    HRESULT hr = GpuCore::Instance().device->CreateCommittedResource(
        &heapProperties, D3D12_HEAP_FLAG_NONE, &txtDesc,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
        IID_PPV_ARGS(texture.GetAddressOf()));

    // 2.upload heap
    const uint64_t uploadBufferSize =
        GetRequiredIntermediateSize(texture.Get(), 0, 1);

    auto heap_property = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto buffer_size = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
    ThrowIfFailed(GpuCore::Instance().device->CreateCommittedResource(
        &heap_property, D3D12_HEAP_FLAG_NONE, &buffer_size,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload)));

    // 3.copy data to upload heap
    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = image->Buffer().data();
    textureData.RowPitch = image->Width() * 4;
    textureData.SlicePitch = textureData.RowPitch * image->Height();

    UpdateSubresources(command_list.Get(), texture.Get(), upload.Get(), 0, 0, 1,
                       &textureData);

    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    command_list->ResourceBarrier(1, &barrier);
}

Texture2D::Texture2D(int width, int height, DXGI_FORMAT format) {

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
    HRESULT hr = GpuCore::Instance().device->CreateCommittedResource(
        &heapProperties, D3D12_HEAP_FLAG_NONE, &txtDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr,
        IID_PPV_ARGS(buffer_.GetAddressOf()));
}

Texture2D::Texture2D(Image image,
                     ComPtr<ID3D12GraphicsCommandList> command_list) {
    CreateTextureHelper(&image, buffer_, command_list);
}

void Texture2D::Allocate(GpuHeap *heap, UINT &index) {
    GpuBuffer::Allocate(heap, index);

    D3D12_SHADER_RESOURCE_VIEW_DESC desc = {
        buffer_->GetDesc().Format, D3D12_SRV_DIMENSION_TEXTURE2D,
        D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING};

    desc.Texture2D.MipLevels = 1;

    GpuCore::Instance().device->CreateShaderResourceView(buffer_.Get(), &desc,
                                                         cpu_handle_);
}
} // namespace core