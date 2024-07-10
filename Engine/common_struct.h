#ifndef _CommonStruct
#define _CommonStruct

#define _USE_MATH_DEFINES
#define WM_MODEL_LOAD (WM_APP + 1)
#define WM_BOX_LOAD (WM_APP + 2)
#define WM_CYLINDER_LOAD (WM_APP + 3)
#define WM_SPHERE_LOAD (WM_APP + 4)

// clang-format off
#include <map>
#include <vector>
#include <windows.h>
#include <string>
#include <iostream>
#include <fstream>		//for text file open
#include <cmath>		//for calculate coordination
#include <omp.h>		//for omp parallel
#include <commdlg.h>	//for file open
#include <wrl.h>
#include <memory>
#include <shellscalingapi.h>

#include <d3d11.h>
#include <directxmath.h>
#include <d3dcompiler.h>
#include <directxtk/SimpleMath.h>

#include <assimp/importer.hpp>
#include <assimp/scene.h>           
#include <assimp/postprocess.h>

// clang-format on

#pragma comment(lib, "shcore.lib")


namespace Engine {
inline void ThrowIfFailed(HRESULT hr) {
    if (FAILED(hr)) {
        throw std::exception();
    }
}

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;
using Microsoft::WRL::ComPtr;

enum EnumShaderType {
    ePhong = 0,
    eCube = 1,
    eImageBased = 2,
    ePhysicallyBased = 3,
    eNormalGeometry = 4,
    eGround = 5,
};

struct Vertex {
    Vector3 position;
    Vector3 normal;
    Vector2 texcoord;
    Vector3 tangent;
};

struct Bone {
    int index;
    std::string name;
    int parent;
    DirectX::XMMATRIX transform;
};

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

struct Material {
    Vector3 ambient = Vector3(0.1f);  // 12
    float shininess = 1.0f;           // 4
    Vector3 diffuse = Vector3(0.5f);  // 12
    float dummy1;                     // 4
    Vector3 specular = Vector3(0.5f); // 12
    float dummy2;                     // 4
};

struct PhysicallyMatrial {
    Vector3 albedo = Vector3(1.0f); // 12
    float roughness = 0.0f;
    float metallic = 0.0f;
    Vector3 dummy;
};

struct Light {
    Vector3 strength = Vector3(1.0f);              // 12
    float fallOffStart = 0.0f;                     // 4
    Vector3 direction = Vector3(0.0f, 0.0f, 1.0f); // 12
    float fallOffEnd = 10.0f;                      // 4
    Vector3 position = Vector3(0.0f, 0.0f, -2.0f); // 12
    float spotPower = 1.0f;                        // 4
};

struct Texture {

};

} // namespace Engine
#endif
