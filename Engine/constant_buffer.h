#ifndef _CONSTANTBUFFER
#define _CONSTANTBUFFER

#include "graphics_manager.h"

namespace engine {

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

__declspec(align(256)) struct MeshConstants {
    Matrix world;
    Matrix worldIT;
    int useHeightMap = 0;
    float heightScale = 0.0f;
    Vector2 dummy;
};

__declspec(align(256)) struct MaterialConstants {

    Vector3 albedoFactor = Vector3(1.0f);
    float roughnessFactor = 1.0f;
    float metallicFactor = 1.0f;
    Vector3 emissionFactor = Vector3(0.0f);

    int useAlbedoMap = 0;
    int useNormalMap = 0;
    int useAOMap = 0;
    int invertNormalMapY = 0;
    int useMetallicMap = 0;
    int useRoughnessMap = 0;
    int useEmissiveMap = 0;
    float dummy = 0.0f;

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

    Vector3 eyeWorld;
    float strengthIBL = 0.0f;

    int textureToDraw = 0; 
    float envLodBias = 0.0f; 
    float lodBias = 2.0f;    
    float dummy2 = 0.0f;

    Light lights[MAX_LIGHTS];
};

// register(b3), PostEffectsPS.hlsl
__declspec(align(256)) struct PostEffectsConstants {
    int mode = 1; // 1: Rendered image, 2: DepthOnly
    float depthScale = 1.0f;
    float fogStrength = 0.0f;
};

} // namespace engine
#endif
