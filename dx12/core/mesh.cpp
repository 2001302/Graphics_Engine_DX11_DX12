#include "mesh.h"

namespace core {
void Mesh::Initialize(const MeshData &mesh_data,
                      ComPtr<ID3D12GraphicsCommandList> command_list,
                      bool use_texture) {

    dx12::Util::CreateVertexBuffer(mesh_data.vertices, vertex_buffer,
                                   vertex_buffer_view);
    index_count = UINT(mesh_data.indices.size());
    vertex_count = UINT(mesh_data.vertices.size());
    stride = UINT(sizeof(Vertex));
    dx12::Util::CreateIndexBuffer(mesh_data.indices, index_buffer,
                                  index_buffer_view);

    if (use_texture) {
        textures[EnumTextureType::ALBEDO] = std::make_shared<Texture>();
        if (textures[EnumTextureType::ALBEDO]->InitAsTexture(
                mesh_data.albedoTextureFilename,
                mesh_data.opacityTextureFilename, false, command_list)) {
        } else if (textures[EnumTextureType::ALBEDO]->InitAsTexture(
                       mesh_data.albedoTextureFilename, false, command_list)) {
        } else {
            std::cout << mesh_data.albedoTextureFilename
                      << " does not exists. Skip texture reading." << std::endl;
        }

        textures[EnumTextureType::NORMAL] = std::make_shared<Texture>();
        if (!textures[EnumTextureType::NORMAL]->InitAsTexture(
                mesh_data.normalTextureFilename, false, command_list)) {
            std::cout << mesh_data.normalTextureFilename
                      << " does not exists. Skip texture reading." << std::endl;
        }

        textures[EnumTextureType::AMBIENT_OCCLUSION] =
            std::make_shared<Texture>();
        if (!textures[EnumTextureType::AMBIENT_OCCLUSION]->InitAsTexture(
                mesh_data.aoTextureFilename, false, command_list)) {
            std::cout << mesh_data.aoTextureFilename
                      << " does not exists. Skip texture reading." << std::endl;
        }

        // Green : Roughness, Blue : Metallic(Metalness)
        textures[EnumTextureType::METALLIC_ROUGHNESS] =
            std::make_shared<Texture>();
        if (!textures[EnumTextureType::METALLIC_ROUGHNESS]
                 ->InitAsMetallicRoughnessTexture(
                     mesh_data.metallicTextureFilename,
                     mesh_data.roughnessTextureFilename, command_list)) {
            std::cout << mesh_data.metallicTextureFilename << " or "
                      << mesh_data.roughnessTextureFilename
                      << " does not exists. Skip texture reading." << std::endl;
        }

        textures[EnumTextureType::EMISSIVE] = std::make_shared<Texture>();
        if (!textures[EnumTextureType::EMISSIVE]->InitAsTexture(
                mesh_data.emissiveTextureFilename, false, command_list)) {
            std::cout << mesh_data.emissiveTextureFilename
                      << " does not exists. Skip texture reading." << std::endl;
        }

        textures[EnumTextureType::HEIGHT] = std::make_shared<Texture>();
        if (!textures[EnumTextureType::HEIGHT]->InitAsTexture(
                mesh_data.heightTextureFilename, false, command_list)) {
            std::cout << mesh_data.heightTextureFilename
                      << " does not exists. Skip texture reading." << std::endl;
        }

        // heap_PS
        D3D12_DESCRIPTOR_HEAP_DESC desc_PS = {};
        desc_PS.NumDescriptors = 5;
        desc_PS.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc_PS.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        dx12::ThrowIfFailed(
            dx12::GpuCore::Instance().device->CreateDescriptorHeap(
                &desc_PS, IID_PPV_ARGS(&heap_PS)));

        // heap_VS
        D3D12_DESCRIPTOR_HEAP_DESC desc_VS = {};
        desc_VS.NumDescriptors = 1;
        desc_VS.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc_VS.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        dx12::ThrowIfFailed(
            dx12::GpuCore::Instance().device->CreateDescriptorHeap(
                &desc_VS, IID_PPV_ARGS(&heap_VS)));

        UINT descriptorSize =
            dx12::GpuCore::Instance().device->GetDescriptorHandleIncrementSize(
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping =
            D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        // srvDesc.Format =
        // textures[EnumTextureType::ALBEDO]->GetDesc().Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        // heapPS
        CD3DX12_CPU_DESCRIPTOR_HANDLE texture_handle_PS(
            heap_PS->GetCPUDescriptorHandleForHeapStart());

        std::vector<Texture *> textures_ps = std::vector<Texture *>{
            textures[EnumTextureType::ALBEDO].get(),
            textures[EnumTextureType::NORMAL].get(),
            textures[EnumTextureType::AMBIENT_OCCLUSION].get(),
            textures[EnumTextureType::METALLIC_ROUGHNESS].get(),
            textures[EnumTextureType::EMISSIVE].get()};

        for (auto resource_ps : textures_ps) {
            if (resource_ps->is_initialized) {
                srvDesc.Format = resource_ps->texture->GetDesc().Format;

                dx12::GpuCore::Instance().device->CreateShaderResourceView(
                    resource_ps->texture.Get(), &srvDesc, texture_handle_PS);
                texture_handle_PS.Offset(descriptorSize);
            } else {
                dx12::GpuCore::Instance().device->CreateShaderResourceView(
                    nullptr, &srvDesc, texture_handle_PS);
                texture_handle_PS.Offset(descriptorSize);
            }
        }

        // heapVS
        CD3DX12_CPU_DESCRIPTOR_HANDLE texture_handle_VS(
            heap_VS->GetCPUDescriptorHandleForHeapStart());

        std::vector<Texture *> textures_vs =
            std::vector<Texture *>{textures[EnumTextureType::HEIGHT].get()};

        for (auto resource_vs : textures_vs) {
            if (resource_vs->is_initialized) {
                srvDesc.Format = resource_vs->texture->GetDesc().Format;

                dx12::GpuCore::Instance().device->CreateShaderResourceView(
                    resource_vs->texture.Get(), &srvDesc, texture_handle_VS);
                texture_handle_VS.Offset(descriptorSize);
            } else {
                dx12::GpuCore::Instance().device->CreateShaderResourceView(
                    nullptr, &srvDesc, texture_handle_VS);
                texture_handle_VS.Offset(descriptorSize);
            }
        }
    }
}
} // namespace core