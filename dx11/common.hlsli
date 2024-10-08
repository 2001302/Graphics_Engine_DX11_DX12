#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

#define MAX_LIGHTS 3 
#define LIGHT_OFF 0x00
#define LIGHT_DIRECTIONAL 0x01
#define LIGHT_POINT 0x02
#define LIGHT_SPOT 0x04
#define LIGHT_SHADOW 0x10

SamplerState linearWrapSampler : register(s0);
SamplerState linearClampSampler : register(s1);
SamplerState shadowPointSampler : register(s2);
SamplerComparisonState shadowCompareSampler : register(s3);
SamplerState pointWrapSampler : register(s4);
SamplerState linearMirrorSampler : register(s5);
SamplerState pointClampSampler : register(s6);

TextureCube envIBLTex : register(t10);
TextureCube specularIBLTex : register(t11);
TextureCube irradianceIBLTex : register(t12);
Texture2D brdfTex : register(t13);

Texture2D shadowMaps[MAX_LIGHTS] : register(t15);
//Texture2D shadowMap1 : register(t16);
//Texture2D shadowMap2 : register(t17);

struct Light
{
    float3 radiance; // Strength
    float fallOffStart;
    float3 direction;
    float fallOffEnd;
    float3 position;
    float spotPower;
    
    uint type;
    float radius;
    float haloRadius;
    float haloStrength;

    matrix viewProj;
    matrix invProj;
};

cbuffer GlobalConstants : register(b0)
{
    matrix view;
    matrix proj;
    matrix invProj;
    matrix viewProj;
    matrix invViewProj;
    matrix invView;

    float3 eyeWorld;
    float strengthIBL;

    int textureToDraw = 0; // 0: Env, 1: Specular, 2: Irradiance
    float envLodBias = 0.0f; 
    float lodBias = 2.0f;
    float globalTime;
    
    Light lights[MAX_LIGHTS];
};

cbuffer MeshConstants : register(b1)
{
    matrix world;
    matrix worldIT; 
    matrix worldInv;
    int useHeightMap;
    float heightScale;
    float windTrunk;
    float windLeaves;
};

cbuffer MaterialConstants : register(b2)
{
    float3 albedoFactor; // baseColor
    float roughnessFactor;
    float metallicFactor;
    float3 emissionFactor;

    int useAlbedoMap;
    int useNormalMap;
    int useAOMap; // Ambient Occlusion
    int invertNormalMapY;
    int useMetallicMap;
    int useRoughnessMap;
    int useEmissiveMap;
    float dummy2;
};

#ifdef SKINNED

// 관절 개수 제약을 없애게 위해 StructuredBuffer 사용
StructuredBuffer<float4x4> boneTransforms : register(t9);

//cbuffer SkinnedConstants : register(b3)
//{
//    float4x4 boneTransforms[52]; // 관절 개수에 따라 조절
//};

#endif

struct VertexShaderInput
{
    float3 posModel : POSITION; 
    float3 normalModel : NORMAL; 
    float2 texcoord : TEXCOORD;
    float3 tangentModel : TANGENT;
    
#ifdef SKINNED
    float4 boneWeights0 : BLENDWEIGHT0;
    float4 boneWeights1 : BLENDWEIGHT1;
    uint4 boneIndices0 : BLENDINDICES0;
    uint4 boneIndices1 : BLENDINDICES1;
#endif
};

struct PixelShaderInput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION0;
    float3 normalWorld : NORMAL0;
    float2 texcoord : TEXCOORD0;
    float3 tangentWorld : TANGENT0;
    float3 posModel : POSITION1; 
};

#endif // __COMMON_HLSLI__