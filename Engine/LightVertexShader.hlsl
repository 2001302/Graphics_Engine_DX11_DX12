cbuffer ModelViewProjectionConstantBuffer : register(b0) {
    matrix model;
    matrix view;
    matrix projection;
};

struct VertexShaderInput {
    float3 pos : POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL;
};

struct PixelShaderInput {
    float4 pos : SV_POSITION;
    float2 texcoord : TEXCOORD;
    float3 normal : NORMAL;
};

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    float4 pos = float4(input.pos, 1.0f);

    pos = mul(pos, model);
    pos = mul(pos, view);
    pos = mul(pos, projection);

    output.pos = pos;

    output.texcoord = input.texcoord;

    output.normal = mul(input.normal, (float3x3)model);
    output.normal = normalize(output.normal);

    return output;
}