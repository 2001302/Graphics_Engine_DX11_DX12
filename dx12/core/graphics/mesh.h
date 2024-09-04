#ifndef _MESH
#define _MESH

#include "command_pool.h"
#include "graphics_util.h"
#include "texture.h"
#include "vertex.h"

namespace dx12 {
enum EnumTextureType {
    ALBEDO = 0,
    NORMAL = 1,
    AMBIENT_OCCLUSION = 2,
    METALLIC_ROUGHNESS = 3,
    EMISSIVE = 4,
    HEIGHT = 5,
    END = 6
};

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
          material_consts_GPU(0), heap_PS(0), heap_VS(0),
          vertex_buffer_view(D3D12_VERTEX_BUFFER_VIEW()),
          index_buffer_view(D3D12_INDEX_BUFFER_VIEW()) {}

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

    std::shared_ptr<Texture> textures[EnumTextureType::END];
    ComPtr<ID3D12DescriptorHeap> heap_PS; // t0 ~ t4
    ComPtr<ID3D12DescriptorHeap> heap_VS; // t0

    void Initialize(const MeshData &mesh_data,
                    ComPtr<ID3D12GraphicsCommandList> command_list,
                    bool use_texture = true);
};

} // namespace core
#endif
