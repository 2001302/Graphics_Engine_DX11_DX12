#ifndef _MESH
#define _MESH

#include "../graphics/graphics_util.h"
#include "vertex.h"

namespace core {
struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<SkinnedVertex> skinnedVertices;
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
    ComPtr<ID3D12Resource> vertexBuffer;
    ComPtr<ID3D12Resource> indexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;

    ComPtr<ID3D12Resource> meshConstsGPU;
    ComPtr<ID3D12Resource> materialConstsGPU;

    ComPtr<ID3D12Resource> albedoTexture;
    ComPtr<ID3D12Resource> emissiveTexture;
    ComPtr<ID3D12Resource> normalTexture;
    ComPtr<ID3D12Resource> heightTexture;
    ComPtr<ID3D12Resource> aoTexture;
    ComPtr<ID3D12Resource> metallicRoughnessTexture;
    ComPtr<ID3D12DescriptorHeap> texture_heap;

    UINT indexCount = 0;
    UINT vertexCount = 0;
    UINT stride = 0;
    UINT offset = 0;
};

} // namespace dx11
#endif
