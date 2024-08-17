Texture2D<float4> inputTex : register(t0);
RWTexture2D<float4> outputTex : register(u0);
SamplerState g_sampler : register(s0);

cbuffer ImageFilterConstData : register(b0)
{
    float dx;
    float dy;
    float threshold;
    float strength;
    float exposure;
    float gamma;
    float option3;
    float option4;
};

struct SamplingPixelShaderInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float3 FilmicToneMapping(float3 color)
{
    color = max(float3(0, 0, 0), color);
    color = (color * (6.2 * color + .5)) / (color * (6.2 * color + 1.7) + 0.06);
    return color;
}

float3 LinearToneMapping(float3 color)
{
    //float3 invGamma = float3(1, 1, 1) / gamma;

    //color = clamp(exposure * color, 0., 1.);
    //color = pow(color, invGamma);
    return color;
}

float3 Uncharted2ToneMapping(float3 color)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2;

    color *= exposure;
    color = ((color * (A * color + C * B) + D * E) /
             (color * (A * color + B) + D * F)) -
            E / F;
    float white =
        ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
    color /= white;
    color = pow(color, float3(1.0, 1.0, 1.0) / gamma);
    return color;
}

float3 lumaBasedReinhardToneMapping(float3 color)
{
    float3 invGamma = float3(1, 1, 1) / gamma;
    float luma = dot(color, float3(0.2126, 0.7152, 0.0722));
    float toneMappedLuma = luma / (1. + luma);
    color *= toneMappedLuma / luma;
    color = pow(color, invGamma);
    return color;
}

[numthreads(256, 1, 1)]
void main(int3 gID : SV_GroupID, int3 gtID : SV_GroupThreadID,
          uint3 dtID : SV_DispatchThreadID)
{
    float dx = 1.0 / 1920;
    float dy = 1.0 / 1080;
    float2 uv = float2((dtID.x + 0.5) * dx, (dtID.y + 0.5) * dy);
    
    float3 color = inputTex.SampleLevel(g_sampler, uv, 0.0).rgb;
    
    // Tone Mapping
    float3 combined = LinearToneMapping(color);
    
    outputTex[dtID.xy] = float4(combined, 1.0f);
}
