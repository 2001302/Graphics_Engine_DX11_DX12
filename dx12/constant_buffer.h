#ifndef _CONSTANTBUFFER
#define _CONSTANTBUFFER

#include "graphics_util.h"

namespace graphics {

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

    Vector3 albedo_factor = Vector3(1.0f);
    float roughness_factor = 1.0f;
    float metallic_factor = 1.0f;
    Vector3 emission_factor = Vector3(0.0f);

    int use_albedo_map = 1;
    int use_normal_map = 1;
    int use_ambient_occlusion_map = 0;
    int invert_normal_map_Y = 0;
    int use_metallic_map = 0;
    int use_roughness_map = 0;
    int use_emissive_map = 0;
    float dummy = 0.0f;
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
    float strengthIBL = 0.5f;

    int textureToDraw = 2;
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
    void Initialize() { Util::CreateConstBuffer(m_cpu, m_gpu); }
    void Upload() { Util::UpdateBuffer(m_cpu, m_gpu); }

  public:
    T_CONSTS &GetCpu() { return m_cpu; }
    const auto Get() { return m_gpu.Get(); }
    const auto GetAddressOf() { return m_gpu.GetAddressOf(); }

    T_CONSTS m_cpu;
    ComPtr<ID3D12Resource> m_gpu;
};

} // namespace core
#endif
