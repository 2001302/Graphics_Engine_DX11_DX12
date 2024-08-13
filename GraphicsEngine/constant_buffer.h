#ifndef _CONSTANTBUFFER
#define _CONSTANTBUFFER

#include "graphics_util.h"

namespace core {

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

__declspec(align(256)) struct MeshConstants {
    Matrix world;
    Matrix worldIT;
    Matrix worldInv;
    int useHeightMap = 0;
    float heightScale = 0.0f;
    float windTrunk = 0.0f;
    float windLeaves = 0.0f;
};

__declspec(align(256)) struct MaterialConstants {

    Vector3 albedoFactor = Vector3(1.0f);
    float roughnessFactor = 1.0f;
    float metallicFactor = 1.0f;
    Vector3 emissionFactor = Vector3(0.0f);

    // 여러 옵션들에 uint 플래그 하나만 사용할 수도 있습니다.
    int useAlbedoMap = 0;
    int useNormalMap = 0;
    int useAOMap = 0;
    int invertNormalMapY = 0;
    int useMetallicMap = 0;
    int useRoughnessMap = 0;
    int useEmissiveMap = 0;
    float dummy = 0.0f;

    // 참고 flags 구현
    /* union {
        uint32_t flags;
        struct {
            // UV0 or UV1 for each texture
            uint32_t baseColorUV : 1;
            uint32_t metallicRoughnessUV : 1;
            uint32_t occlusionUV : 1;
            uint32_t emissiveUV : 1;
            uint32_t normalUV : 1;

            // Three special modes
            uint32_t twoSided : 1;
            uint32_t alphaTest : 1;
            uint32_t alphaBlend : 1;

            uint32_t _pad : 8;

            uint32_t alphaRef : 16; // half float
        };
    };*/
};

struct Light {
    Vector3 radiance = Vector3(5.0f); // strength
    float fallOffStart = 0.0f;
    Vector3 direction = Vector3(0.0f, 0.0f, 1.0f);
    float fallOffEnd = 10.0f;
    Vector3 position = Vector3(0.0f, 0.0f, -2.0f);
    float spotPower = 1.0f;

    // Light type bitmasking
    // ex) LIGHT_SPOT | LIGHT_SHADOW
    uint32_t type = LIGHT_OFF;
    float radius = 0.0f;

    float haloRadius = 0.0f;
    float haloStrength = 0.0f;

    Matrix viewProj;
    Matrix invProj;
};

// register(b1)
__declspec(align(256)) struct GlobalConstants {
    Matrix view;
    Matrix proj;
    Matrix invProj;
    Matrix viewProj;
    Matrix invViewProj;
    Matrix invView;

    Vector3 eyeWorld;
    float strengthIBL = 0.0f;

    int textureToDraw = 0;
    float envLodBias = 0.0f;
    float lodBias = 2.0f;
    float dummy2 = 0.0f;

    Light lights[MAX_LIGHTS];
};

__declspec(align(256)) struct PostEffectsConstants {
    int mode = 1; // 1: Rendered image, 2: DepthOnly
    float depthScale = 1.0f;
    float fogStrength = 0.0f;
};

__declspec(align(256)) struct VolumeConsts {
    Vector3 uvwOffset = Vector3(0.0f);
    float lightAbsorption = 5.0f;
    Vector3 lightDir = Vector3(0.0f, 1.0f, 0.0f);
    float densityAbsorption = 10.0f;
    Vector3 lightColor = Vector3(1, 1, 1) * 40.0f;
    float aniso = 0.3f;
};

template <typename T_CONSTS> class ConstantBuffer {
  public:
    void Initialize() { GraphicsUtil::CreateConstBuffer(m_cpu, m_gpu); }

    void Upload() { GraphicsUtil::UpdateBuffer(m_cpu, m_gpu); }

  public:
    T_CONSTS &GetCpu() { return m_cpu; }
    const auto Get() { return m_gpu.Get(); }
    const auto GetAddressOf() { return m_gpu.GetAddressOf(); }

    T_CONSTS m_cpu;
    ComPtr<ID3D11Buffer> m_gpu;
};

} // namespace core
#endif
