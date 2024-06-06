#include "Common.hlsli" // 쉐이더에서도 include 사용 가능

Texture2D g_texture0 : register(t0);
SamplerState g_sampler : register(s0);

cbuffer PixelConstantBuffer : register(b0)
{
    float3 eyeWorld;
    bool useTexture;
    Material material;
    Light light[MAX_LIGHTS];
    bool useBlinnPhong;
    float3 dummy;
};

float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal,
    float3 toEye, Material mat)
{
    if (useBlinnPhong)
    {
        float3 halfway = normalize(toEye + lightVec);
        float hdotn = dot(halfway, normal);
        float3 specular = mat.specular * pow(max(hdotn, 0.0f), mat.shininess * 2.0);
        return mat.ambient + (mat.diffuse + specular) * lightStrength;
    }
    else
    {
        float3 r = -reflect(lightVec, normal);
        float3 specular = mat.specular * pow(max(dot(toEye, r), 0.0f), mat.shininess);
        return mat.ambient + (mat.diffuse + specular) * lightStrength;
    }
}

float3 ComputeDirectionalLight(Light L, Material mat, float3 normal,
    float3 toEye)
{
    float3 lightVec = -L.direction;

    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.strength * ndotl;

    // Luna DX12 책에서는 Specular 계산에도
    // Lambert's law가 적용된 lightStrength를 사용합니다.
    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    // Linear falloff
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

float3 ComputePointLight(Light L, Material mat, float3 pos, float3 normal,
    float3 toEye)
{
    float3 lightVec = L.position - pos;

    // 쉐이딩할 지점부터 조명까지의 거리 계산
    float d = length(lightVec);

    // 너무 멀면 조명이 적용되지 않음
    if (d > L.fallOffEnd)
    {
        return float3(0.0, 0.0, 0.0);
    }
    else
    {
        lightVec /= d;

        float ndotl = max(dot(lightVec, normal), 0.0f);
        float3 lightStrength = L.strength * ndotl;

        float att = CalcAttenuation(d, L.fallOffStart, L.fallOffEnd);
        lightStrength *= att;

        return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
    }
}

float3 ComputeSpotLight(Light L, Material mat, float3 pos, float3 normal,
    float3 toEye)
{
    float3 lightVec = L.position - pos;

    // 쉐이딩할 지점부터 조명까지의 거리 계산
    float d = length(lightVec);

    // 너무 멀면 조명이 적용되지 않음
    if (d > L.fallOffEnd)
    {
        return float3(0.0f, 0.0f, 0.0f);
    }
    else
    {
        lightVec /= d;

        float ndotl = max(dot(lightVec, normal), 0.0f);
        float3 lightStrength = L.strength * ndotl;

        float att = CalcAttenuation(d, L.fallOffStart, L.fallOffEnd);
        lightStrength *= att;

        float spotFactor = pow(max(-dot(lightVec, L.direction), 0.0f), L.spotPower);
        lightStrength *= spotFactor;

        return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
    }

    // if에 else가 없을 경우 경고 발생
    // warning X4000: use of potentially uninitialized variable
}

float4 main(PixelShaderInput input) : SV_TARGET
{

    float3 toEye = normalize(eyeWorld - input.posWorld);

    float3 color = float3(0.0, 0.0, 0.0);

    int i = 0;

    // https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-for
    // https://forum.unity.com/threads/what-are-unroll-and-loop-when-to-use-them.1283096/

    [unroll] // warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
    for (i = 0; i < NUM_DIR_LIGHTS; ++i)
    {
        color += ComputeDirectionalLight(light[i], material, input.normalWorld, toEye);
    }

    [unroll]
    for (i = NUM_DIR_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS; ++i)
    {
        color += ComputePointLight(light[i], material, input.posWorld, input.normalWorld, toEye);
    }

    [unroll]
    for (i = NUM_DIR_LIGHTS + NUM_POINT_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS; ++i)
    {
        color += ComputeSpotLight(light[i], material, input.posWorld, input.normalWorld, toEye);
    }

    return useTexture ? float4(color, 1.0) * g_texture0.Sample(g_sampler, input.texcoord) : float4(color, 1.0);
}
