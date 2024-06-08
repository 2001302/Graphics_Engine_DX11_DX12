#include "Common.hlsli"

cbuffer LightVertexConstantBuffer : register(b0) {
    matrix model;
    matrix view;
    matrix projection;
};

struct LightPixelShaderInput {
    float4 pos : SV_POSITION;
    float2 texcoord : TEXCOORD;
    float3 normal : NORMAL;
};

LightPixelShaderInput main(VertexShaderInput input)
{
    LightPixelShaderInput output;
    float4 pos = float4(input.posModel, 1.0f);

    pos = mul(pos, model);
    pos = mul(pos, view);
    pos = mul(pos, projection);

    output.pos = pos;

    output.texcoord = input.texcoord;

    output.normal = mul(input.normalModel, (float3x3)model);
    output.normal = normalize(output.normal);

    return output;
}