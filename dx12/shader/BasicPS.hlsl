#include "Common.hlsli"
#include "DiskSamples.hlsli"

Texture2D albedoTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D aoTex : register(t2);
Texture2D metallicRoughnessTex : register(t3);
Texture2D emissiveTex : register(t4);

static const float3 Fdielectric = 0.04;

float3 SchlickFresnel(float3 F0, float NdotH) {
    return F0 + (1.0 - F0) * pow(2.0, (-5.55473 * NdotH - 6.98316) * NdotH);
}

struct PixelShaderOutput {
    float4 pixelColor : SV_Target0;
};

float3 GetNormal(PixelShaderInput input) {
    float3 normalWorld = normalize(input.normalWorld);

    if (useNormalMap) {
        float3 normal =
            normalTex.SampleLevel(linearWrapSampler, input.texcoord, lodBias)
                .rgb;
        normal = 2.0 * normal - 1.0; //[-1.0, 1.0]

        normal.y = invertNormalMapY ? -normal.y : normal.y;

        float3 N = normalWorld;
        float3 T =
            normalize(input.tangentWorld - dot(input.tangentWorld, N) * N);
        float3 B = cross(N, T);

        float3x3 TBN = float3x3(T, B, N);
        normalWorld = normalize(mul(normal, TBN));
    }

    return normalWorld;
}

float3 DiffuseIBL(float3 albedo, float3 normalWorld, float3 pixelToEye,
                  float metallic) {
    float3 F0 = lerp(Fdielectric, albedo, metallic);
    float3 F = SchlickFresnel(F0, max(0.0, dot(normalWorld, pixelToEye)));
    float3 kd = lerp(1.0 - F, 0.0, metallic);
    float3 irradiance =
        irradianceIBLTex.SampleLevel(linearWrapSampler, normalWorld, 0).rgb;

    return kd * albedo * irradiance;
}

float3 SpecularIBL(float3 albedo, float3 normalWorld, float3 pixelToEye,
                   float metallic, float roughness) {
    float2 specularBRDF =
        brdfTex
            .SampleLevel(linearClampSampler,
                         float2(dot(normalWorld, pixelToEye), 1.0 - roughness),
                         0.0f)
            .rg;
    float3 specularIrradiance =
        specularIBLTex
            .SampleLevel(linearWrapSampler, reflect(-pixelToEye, normalWorld),
                         2 + roughness * 5.0f)
            .rgb;
    const float3 Fdielectric = 0.04;
    float3 F0 = lerp(Fdielectric, albedo, metallic);

    return (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;
}

float3 AmbientLightingByIBL(float3 albedo, float3 normalW, float3 pixelToEye,
                            float ao, float metallic, float roughness) {
    float3 diffuseIBL = DiffuseIBL(albedo, normalW, pixelToEye, metallic);
    float3 specularIBL =
        SpecularIBL(albedo, normalW, pixelToEye, metallic, roughness);

    return (diffuseIBL + specularIBL) * ao;
}

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float NdfGGX(float NdotH, float roughness, float alphaPrime) {
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;
    float denom = (NdotH * NdotH) * (alphaSq - 1.0) + 1.0;
    return alphaPrime * alphaPrime / (3.141592 * denom * denom);
}

// Single term for separable Schlick-GGX below.
float SchlickG1(float NdotV, float k) {
    return NdotV / (NdotV * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's
// method.
float SchlickGGX(float NdotI, float NdotO, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return SchlickG1(NdotI, k) * SchlickG1(NdotO, k);
}

float random(float3 seed, int i) {
    float4 seed4 = float4(seed, i);
    float dot_product = dot(seed4, float4(12.9898, 78.233, 45.164, 94.673));
    return frac(sin(dot_product) * 43758.5453);
}

// NdcDepthToViewDepth
float N2V(float ndcDepth, matrix invProj) {
    float4 pointView = mul(float4(0, 0, ndcDepth, 1), invProj);
    return pointView.z / pointView.w;
}

#define NEAR_PLANE 0.1
// #define LIGHT_WORLD_RADIUS 0.001
#define LIGHT_FRUSTUM_WIDTH 0.34641

float PCF_Filter(float2 uv, float zReceiverNdc, float filterRadiusUV,
                 Texture2D shadowMap) {
    float sum = 0.0f;
    for (int i = 0; i < 64; ++i) {
        float2 offset = diskSamples64[i] * filterRadiusUV;
        sum += shadowMap.SampleCmpLevelZero(shadowCompareSampler, uv + offset,
                                            zReceiverNdc);
    }
    return sum / 64;
}

void FindBlocker(out float avgBlockerDepthView, out float numBlockers,
                 float2 uv, float zReceiverView, Texture2D shadowMap,
                 matrix invProj, float lightRadiusWorld) {
    float lightRadiusUV = lightRadiusWorld / LIGHT_FRUSTUM_WIDTH;

    float searchRadius =
        lightRadiusUV * (zReceiverView - NEAR_PLANE) / zReceiverView;

    float blockerSum = 0;
    numBlockers = 0;
    for (int i = 0; i < 64; ++i) {
        float shadowMapDepth =
            shadowMap
                .SampleLevel(shadowPointSampler,
                             float2(uv + diskSamples64[i] * searchRadius), 0)
                .r;

        shadowMapDepth = N2V(shadowMapDepth, invProj);

        if (shadowMapDepth < zReceiverView) {
            blockerSum += shadowMapDepth;
            numBlockers++;
        }
    }
    avgBlockerDepthView = blockerSum / numBlockers;
}

float PCSS(float2 uv, float zReceiverNdc, Texture2D shadowMap, matrix invProj,
           float lightRadiusWorld) {
    float lightRadiusUV = lightRadiusWorld / LIGHT_FRUSTUM_WIDTH;

    float zReceiverView = N2V(zReceiverNdc, invProj);

    // STEP 1: blocker search
    float avgBlockerDepthView = 0;
    float numBlockers = 0;

    FindBlocker(avgBlockerDepthView, numBlockers, uv, zReceiverView, shadowMap,
                invProj, lightRadiusWorld);

    if (numBlockers < 1) {
        // There are no occluders so early out(this saves filtering)
        return 1.0f;
    } else {
        // STEP 2: penumbra size
        float penumbraRatio =
            (zReceiverView - avgBlockerDepthView) / avgBlockerDepthView;
        float filterRadiusUV =
            penumbraRatio * lightRadiusUV * NEAR_PLANE / zReceiverView;

        // STEP 3: filtering
        return PCF_Filter(uv, zReceiverNdc, filterRadiusUV, shadowMap);
    }
}

float3 LightRadiance(Light light, float3 representativePoint, float3 posWorld,
                     float3 normalWorld, Texture2D shadowMap) {
    // Directional light
    float3 lightVec =
        light.type & LIGHT_DIRECTIONAL
            ? -light.direction
            : representativePoint - posWorld; //: light.position - posWorld;

    float lightDist = length(lightVec);
    lightVec /= lightDist;

    // Spot light
    float spotFator =
        light.type & LIGHT_SPOT
            ? pow(max(-dot(lightVec, light.direction), 0.0f), light.spotPower)
            : 1.0f;

    // Distance attenuation
    float att = saturate((light.fallOffEnd - lightDist) /
                         (light.fallOffEnd - light.fallOffStart));

    // Shadow map
    float shadowFactor = 1.0;

    if (light.type & LIGHT_SHADOW) {
        const float nearZ = 0.01; // camera setting

        float4 lightScreen = mul(float4(posWorld, 1.0), light.viewProj);
        lightScreen.xyz /= lightScreen.w;

        float2 lightTexcoord = float2(lightScreen.x, -lightScreen.y);
        lightTexcoord += 1.0;
        lightTexcoord *= 0.5;

        uint width, height, numMips;
        shadowMap.GetDimensions(0, width, height, numMips);

        float radiusScale = 0.5;
        shadowFactor = PCSS(lightTexcoord, lightScreen.z - 0.001, shadowMap,
                            light.invProj, light.radius * radiusScale);
    }

    float3 radiance = light.radiance * spotFator * att * shadowFactor;

    return radiance;
}

PixelShaderOutput main(PixelShaderInput input) {
    float3 pixelToEye = normalize(eyeWorld - input.posWorld);
    float3 normalWorld = GetNormal(input);

    float4 albedo = useAlbedoMap
                        ? albedoTex.SampleLevel(linearWrapSampler,
                                                input.texcoord, lodBias) *
                              float4(albedoFactor, 1)
                        : float4(albedoFactor, 1);

    clip(albedo.a - 0.5); // Tree leaves

    float ao =
        useAOMap
            ? aoTex.SampleLevel(linearWrapSampler, input.texcoord, lodBias).r
            : 1.0;
    float metallic =
        useMetallicMap
            ? metallicRoughnessTex
                      .SampleLevel(linearWrapSampler, input.texcoord, lodBias)
                      .b *
                  metallicFactor
            : metallicFactor;
    float roughness =
        useRoughnessMap
            ? metallicRoughnessTex
                      .SampleLevel(linearWrapSampler, input.texcoord, lodBias)
                      .g *
                  roughnessFactor
            : roughnessFactor;
    float3 emission = useEmissiveMap ? emissiveTex
                                           .SampleLevel(linearWrapSampler,
                                                        input.texcoord, lodBias)
                                           .rgb
                                     : emissionFactor;

    float3 ambientLighting =
        AmbientLightingByIBL(albedo.rgb, normalWorld, pixelToEye, ao, metallic,
                             roughness) *
        strengthIBL;

    float3 directLighting = float3(0, 0, 0);

    [unroll] // warning X3550: sampler array index must be a literal expression,
             // forcing loop to unroll
        for (int i = 0; i < MAX_LIGHTS; ++i) {
        if (lights[i].type) {
            float3 L = lights[i].position - input.posWorld;
            float3 r =
                normalize(reflect(eyeWorld - input.posWorld, normalWorld));
            float3 centerToRay = dot(L, r) * r - L;
            float3 representativePoint =
                L + centerToRay *
                        clamp(lights[i].radius / length(centerToRay), 0.0, 1.0);
            representativePoint += input.posWorld;
            float3 lightVec = representativePoint - input.posWorld;

            // float3 lightVec = lights[i].position - input.posWorld;
            float lightDist = length(lightVec);
            lightVec /= lightDist;
            float3 halfway = normalize(pixelToEye + lightVec);

            float NdotI = max(0.0, dot(normalWorld, lightVec));
            float NdotH = max(0.0, dot(normalWorld, halfway));
            float NdotO = max(0.0, dot(normalWorld, pixelToEye));

            const float3 Fdielectric = 0.04;
            float3 F0 = lerp(Fdielectric, albedo.rgb, metallic);
            float3 F = SchlickFresnel(F0, max(0.0, dot(halfway, pixelToEye)));
            float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metallic);
            float3 diffuseBRDF = kd * albedo.rgb;

            // Sphere Normalization
            float alpha = roughness * roughness;
            float alphaPrime =
                saturate(alpha + lights[i].radius / (2.0 * lightDist));

            float D = NdfGGX(NdotH, roughness, alphaPrime);
            float3 G = SchlickGGX(NdotI, NdotO, roughness);
            float3 specularBRDF = (F * D * G) / max(1e-5, 4.0 * NdotI * NdotO);

            float3 radiance =
                LightRadiance(lights[i], representativePoint, input.posWorld,
                              normalWorld, shadowMaps[i]);

            if (abs(dot(float3(1, 1, 1), radiance)) > 1e-5)
                directLighting +=
                    (diffuseBRDF + specularBRDF) * radiance * NdotI;
        }
    }

    PixelShaderOutput output;
    output.pixelColor =
        float4(ambientLighting + directLighting + emission, 1.0);
    output.pixelColor = clamp(output.pixelColor, 0.0, 1000.0);

    return output;
}
