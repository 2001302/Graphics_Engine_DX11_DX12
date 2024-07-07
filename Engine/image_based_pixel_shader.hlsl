#include "common.hlsli"

Texture2D g_texture0 : register(t0);
TextureCube g_specularCube : register(t1);
TextureCube g_diffuseCube : register(t2);
SamplerState g_sampler : register(s0);

cbuffer ImageBasedPixelConstantBuffer : register(b0) {
    float3 eyeWorld;
    bool useTexture;
    Material material;
};

float4 main(PixelShaderInput input) : SV_TARGET {

    float3 toEye = normalize(eyeWorld - input.posWorld);

    float4 diffuse = g_diffuseCube.Sample(g_sampler, input.normalWorld);
    float4 specular =
        g_specularCube.Sample(g_sampler, reflect(toEye, input.normalWorld));

    specular *=
        pow((specular.x + specular.y + specular.z) / 3.0, material.shininess);

    diffuse.xyz *= material.diffuse;
    specular.xyz *= material.specular;

    if (useTexture) {
        diffuse *= g_texture0.Sample(g_sampler, input.texcoord);
    }

    return diffuse + specular;
}
