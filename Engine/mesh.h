#ifndef _MESH
#define _MESH

#include "direct3D.h"
#include "vertex.h"

namespace dx11 {

struct Mesh {
    std::string name;
    int boneIndex;
    std::string materialName;
    std::string textureFilename;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11ShaderResourceView> textureResourceView;
    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11Buffer> indexBuffer;

    ComPtr<ID3D11Texture2D> albedoTexture;
    ComPtr<ID3D11Texture2D> emissiveTexture;
    ComPtr<ID3D11Texture2D> normalTexture;
    ComPtr<ID3D11Texture2D> heightTexture;
    ComPtr<ID3D11Texture2D> aoTexture;
    ComPtr<ID3D11Texture2D> metallicTexture;
    ComPtr<ID3D11Texture2D> roughnessTexture;

    std::string albedoTextureFilename;
    std::string emissiveTextureFilename;
    std::string normalTextureFilename;
    std::string heightTextureFilename;
    std::string aoTextureFilename; // Ambient Occlusion
    std::string metallicTextureFilename;
    std::string roughnessTextureFilename;

    ComPtr<ID3D11ShaderResourceView> albedoSRV;
    ComPtr<ID3D11ShaderResourceView> emissiveSRV;
    ComPtr<ID3D11ShaderResourceView> normalSRV;
    ComPtr<ID3D11ShaderResourceView> heightSRV;
    ComPtr<ID3D11ShaderResourceView> aoSRV;
    ComPtr<ID3D11ShaderResourceView> metallicSRV;
    ComPtr<ID3D11ShaderResourceView> roughnessSRV;
};

} // namespace dx11
#endif
