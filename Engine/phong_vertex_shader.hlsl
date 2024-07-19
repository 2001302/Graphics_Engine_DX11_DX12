#include "common.hlsli"

cbuffer VertexConstantBuffer : register(b0)
{
    matrix model;
    matrix invTranspose;
    matrix view;
    matrix projection;
};

PixelShaderInput main(VertexShaderInput input)
{
    //The model matrix transforms the object from its local origin to its position in world space.
    // ¸ðµ¨ ÁÂÇ¥°èÀÇ À§Ä¡ -> [¸ðµ¨ Çà·Ä °öÇÏ±â] -> ¿ùµå ÁÂÇ¥°èÀÇ À§Ä¡
    // -> [ºä Çà·Ä °öÇÏ±â] -> ºä ÁÂÇ¥°èÀÇ À§Ä¡ -> [ÇÁ·ÎÁ§¼Ç Çà·Ä °öÇÏ±â]
    // -> ½ºÅ©¸° ÁÂÇ¥°èÀÇ À§Ä¡

    //Since the view coordinate system is NDC, lighting calculations are done using world coordinates.

    PixelShaderInput output;
    float4 pos = float4(input.posModel, 1.0f);
    pos = mul(pos, model);

    output.posWorld = pos.xyz; //Store World Position Separately

    pos = mul(pos, view);
    pos = mul(pos, projection);

    output.posProj = pos;
    output.texcoord = input.texcoord;
    // output.color = input.color;

    float4 normal = float4(input.normalModel, 0.0f);
    output.normalWorld = mul(normal, invTranspose).xyz;
    output.normalWorld = normalize(output.normalWorld);

    return output;
}
