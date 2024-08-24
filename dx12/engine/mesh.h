#ifndef _MESH
#define _MESH

#include "../graphics/graphics_util.h"
#include "vertex.h"
//#include "texture_3d.h"

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
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    ComPtr<ID3D12Resource> indexBuffer;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;

    ComPtr<ID3D12Resource> meshConstsGPU;
    ComPtr<ID3D12Resource> materialConstsGPU;

    ComPtr<ID3D12Resource> albedoTexture;
    ComPtr<ID3D12Resource> emissiveTexture;
    ComPtr<ID3D12Resource> normalTexture;
    ComPtr<ID3D12Resource> heightTexture;
    ComPtr<ID3D12Resource> aoTexture;
    ComPtr<ID3D12Resource> metallicRoughnessTexture;

    ComPtr<ID3D12Resource> albedoSRV;
    ComPtr<ID3D12Resource> emissiveSRV;
    ComPtr<ID3D12Resource> normalSRV;
    ComPtr<ID3D12Resource> heightSRV;
    ComPtr<ID3D12Resource> aoSRV;
    ComPtr<ID3D12Resource> metallicRoughnessSRV;

    ComPtr<ID3D12DescriptorHeap> texture_heap;

    //// 3D Textures
    //Texture3D densityTex;
    //Texture3D lightingTex;

    UINT indexCount = 0;
    UINT vertexCount = 0;
    UINT stride = 0;
    UINT offset = 0;
};

} // namespace dx11
#endif
