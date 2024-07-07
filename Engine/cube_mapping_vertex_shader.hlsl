#include "Common.hlsli"

cbuffer BasicVertexConstantBuffer : register(b0) {
    matrix model;
    matrix invTranspose;
    matrix view;
    matrix projection;
};

struct CubeMappingPixelShaderInput {
    float4 posProj : SV_POSITION;
    float3 posModel : POSITION;
};

CubeMappingPixelShaderInput main(VertexShaderInput input) {

    CubeMappingPixelShaderInput output;
    float4 pos = float4(input.posModel, 1.0f);

    pos = mul(pos, model); // Identity
    pos = mul(pos, view);
    pos = mul(pos, projection);

    output.posProj = pos;
    output.posModel = input.posModel;

    return output;
}
