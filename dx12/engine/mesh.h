#ifndef _MESH
#define _MESH

#include "../graphics/command_pool.h"
#include "../graphics/graphics_util.h"
#include "texture.h"
#include "vertex.h"

using namespace std;
using namespace DirectX;

namespace core {
struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<SkinnedVertex> skinned_vertices;
    std::vector<uint32_t> indices;
    std::string albedoTextureFilename;
    std::string emissiveTextureFilename;
    std::string normalTextureFilename;
    std::string heightTextureFilename;
    std::string aoTextureFilename; // Ambient Occlusion
    std::string metallicTextureFilename;
    std::string roughnessTextureFilename;
    std::string opacityTextureFilename;
};

struct Mesh {
    Mesh()
        : vertex_buffer(0), index_buffer(0), mesh_consts_GPU(0),
          material_consts_GPU(0), heap_PS(0), heap_VS(0) {}

    ComPtr<ID3D12Resource> vertex_buffer;
    ComPtr<ID3D12Resource> index_buffer;
    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;
    D3D12_INDEX_BUFFER_VIEW index_buffer_view;
    UINT index_count = 0;
    UINT vertex_count = 0;
    UINT stride = 0;
    UINT offset = 0;

    ComPtr<ID3D12Resource> mesh_consts_GPU;
    ComPtr<ID3D12Resource> material_consts_GPU;

    std::shared_ptr<Texture> albedo_texture;
    std::shared_ptr<Texture> emissive_texture;
    std::shared_ptr<Texture> normal_texture;
    std::shared_ptr<Texture> ao_texture;
    std::shared_ptr<Texture> metallic_roughness_texture;
    std::shared_ptr<Texture> height_texture;

    ComPtr<ID3D12DescriptorHeap> heap_PS; // t0 ~ t4
    ComPtr<ID3D12DescriptorHeap> heap_VS; // t0

    void Initialize(const MeshData &mesh_data,
                    ComPtr<ID3D12GraphicsCommandList> command_list) {

        dx12::Util::CreateVertexBuffer(mesh_data.vertices, vertex_buffer,
                                       vertex_buffer_view);
        index_count = UINT(mesh_data.indices.size());
        vertex_count = UINT(mesh_data.vertices.size());
        stride = UINT(sizeof(Vertex));
        dx12::Util::CreateIndexBuffer(mesh_data.indices, index_buffer,
                                      index_buffer_view);

        albedo_texture = std::make_shared<Texture>();
        if (albedo_texture->InitAsTexture(mesh_data.albedoTextureFilename,
                                          mesh_data.opacityTextureFilename,
                                          false, command_list)) {
        } else if (albedo_texture->InitAsTexture(
                       mesh_data.albedoTextureFilename, false, command_list)) {
        } else {
            cout << mesh_data.albedoTextureFilename
                 << " does not exists. Skip texture reading." << endl;
        }

        normal_texture = std::make_shared<Texture>();
        if (!normal_texture->InitAsTexture(mesh_data.normalTextureFilename,
                                           false, command_list)) {
            cout << mesh_data.normalTextureFilename
                 << " does not exists. Skip texture reading." << endl;
        }

        ao_texture = std::make_shared<Texture>();
        if (!ao_texture->InitAsTexture(mesh_data.aoTextureFilename, false,
                                       command_list)) {
            cout << mesh_data.aoTextureFilename
                 << " does not exists. Skip texture reading." << endl;
        }

        // Green : Roughness, Blue : Metallic(Metalness)
        metallic_roughness_texture = std::make_shared<Texture>();
        if (!metallic_roughness_texture->InitAsMetallicRoughnessTexture(
                mesh_data.metallicTextureFilename,
                mesh_data.roughnessTextureFilename, command_list)) {
            cout << mesh_data.metallicTextureFilename << " or "
                 << mesh_data.roughnessTextureFilename
                 << " does not exists. Skip texture reading." << endl;
        }

        emissive_texture = std::make_shared<Texture>();
        if (!emissive_texture->InitAsTexture(mesh_data.emissiveTextureFilename,
                                             false, command_list)) {
            cout << mesh_data.emissiveTextureFilename
                 << " does not exists. Skip texture reading." << endl;
        }

        height_texture = std::make_shared<Texture>();
        if (!height_texture->InitAsTexture(mesh_data.heightTextureFilename,
                                           false, command_list)) {
            cout << mesh_data.heightTextureFilename
                 << " does not exists. Skip texture reading." << endl;
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
        // srvDesc.Format = albedo_texture->GetDesc().Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        // heapPS
        CD3DX12_CPU_DESCRIPTOR_HANDLE texture_handle_PS(
            heap_PS->GetCPUDescriptorHandleForHeapStart());

        std::vector<Texture *> textures_ps = std::vector<Texture *>{
            albedo_texture.get(), normal_texture.get(), ao_texture.get(),
            metallic_roughness_texture.get(), emissive_texture.get()};

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
            std::vector<Texture *>{height_texture.get()};

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
};

} // namespace core
#endif
