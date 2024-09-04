Texture2D g_texture0 : register(t0);
SamplerState g_sampler : register(s0);

cbuffer ImageFilterConstData : register(b1) {
    float threshold;
    float strength;
    float exposure;
    float gamma;
};

struct SamplingPixelShaderInput {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float3 FilmicToneMapping(float3 color) {
    color = max(float3(0, 0, 0), color);
    color = (color * (6.2 * color + .5)) / (color * (6.2 * color + 1.7) + 0.06);
    return color;
}

float3 LinearToneMapping(float3 color) {
    float3 invGamma = float3(1, 1, 1) / gamma;

    color = clamp(exposure * color, 0., 1.);
    color = pow(color, invGamma);
    return color;
}

float3 Uncharted2ToneMapping(float3 color) {
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

float3 lumaBasedReinhardToneMapping(float3 color) {
    float3 invGamma = float3(1, 1, 1) / gamma;
    float luma = dot(color, float3(0.2126, 0.7152, 0.0722));
    float toneMappedLuma = luma / (1. + luma);
    color *= toneMappedLuma / luma;
    color = pow(color, invGamma);
    return color;
}

float4 main(SamplingPixelShaderInput input) : SV_TARGET {
    float3 color0 = g_texture0.Sample(g_sampler, input.texcoord).rgb;

    // Tone Mapping
    float3 combined = LinearToneMapping(color0);

    return float4(combined, 1.0f);
}
