#ifndef _MESH
#define _MESH

#include "../graphics/graphics_util.h"
#include "vertex.h"
#include "texture.h"

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
          material_consts_GPU(0), heap_PS(0), heap_VS(0) {

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
    }

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
    ComPtr<ID3D12DescriptorHeap> heap_PS; // t0 ~ t4

    std::shared_ptr<Texture> height_texture;
    ComPtr<ID3D12DescriptorHeap> heap_VS; // t0
};

} // namespace core
#endif
