#include "Common.hlsli" 

Texture2D g_heightTexture : register(t0);

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    
#ifdef SKINNED
    
    float weights[8];
    weights[0] = input.boneWeights0.x;
    weights[1] = input.boneWeights0.y;
    weights[2] = input.boneWeights0.z;
    weights[3] = input.boneWeights0.w;
    weights[4] = input.boneWeights1.x;
    weights[5] = input.boneWeights1.y;
    weights[6] = input.boneWeights1.z;
    weights[7] = input.boneWeights1.w;
    
    uint indices[8]; 
    indices[0] = input.boneIndices0.x;
    indices[1] = input.boneIndices0.y;
    indices[2] = input.boneIndices0.z;
    indices[3] = input.boneIndices0.w;
    indices[4] = input.boneIndices1.x;
    indices[5] = input.boneIndices1.y;
    indices[6] = input.boneIndices1.z;
    indices[7] = input.boneIndices1.w;

    float3 posModel = float3(0.0f, 0.0f, 0.0f);
    float3 normalModel = float3(0.0f, 0.0f, 0.0f);
    float3 tangentModel = float3(0.0f, 0.0f, 0.0f);
    
    for(int i = 0; i < 8; ++i)
    {
        float4x4 boneTransform = boneTransforms[indices[i]];
        posModel += weights[i] * mul(float4(input.posModel, 1.0f), boneTransform).xyz;
        normalModel += weights[i] * normalize(mul(input.normalModel, (float3x3) boneTransform));
        tangentModel += weights[i] * normalize(mul(input.tangentModel, (float3x3) boneTransform));
    }

    input.posModel = posModel;
    input.normalModel = normalModel;
    input.tangentModel = tangentModel;

#endif

    if (windTrunk != 0.0)
    {
        float2 rotCenter = float2(0.0f, -0.5f);
        float2 temp = (input.posModel.xy - rotCenter);
        float coeff = windTrunk * pow(max(0, temp.y), 2.0) * sin(globalTime);
        
        float2x2 rot = float2x2(cos(coeff), sin(coeff), -sin(coeff), cos(coeff));
        
        input.posModel.xy = mul(temp, rot);
        input.posModel.xy += rotCenter;
    }
    
    if (windLeaves != 0.0)
    {
        float3 windVel = float3(sin(input.posModel.x * 100.0 + globalTime * 0.1)
                                * cos(input.posModel.y * 100 + globalTime * 0.1), 0, 0)
                                * sin(globalTime * 10.0);
    }
    
    output.posModel = input.posModel;
    output.normalWorld = mul(float4(input.normalModel, 0.0f), worldIT).xyz;
    output.normalWorld = normalize(output.normalWorld);
    output.posWorld = mul(float4(input.posModel, 1.0f), world).xyz;

    if (useHeightMap)
    {
        float height = g_heightTexture.SampleLevel(linearClampSampler, input.texcoord, 0).r;
        height = height * 2.0 - 1.0;
        output.posWorld += output.normalWorld * height * heightScale;
    }

    output.posProj = mul(float4(output.posWorld, 1.0), viewProj);
    output.texcoord = input.texcoord;
    output.tangentWorld = mul(float4(input.tangentModel, 0.0f), world).xyz;

    return output;
}
