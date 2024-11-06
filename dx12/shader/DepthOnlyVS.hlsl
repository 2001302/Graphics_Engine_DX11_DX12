#include "Common.hlsli"

float4 main(VertexShaderInput input) : SV_POSITION {

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

    for (int i = 0; i < 8; ++i) {
        float4x4 boneTransform = boneTransforms[indices[i]];
        posModel +=
            weights[i] * mul(float4(input.posModel, 1.0f), boneTransform).xyz;
    }

    input.posModel = posModel;

#endif

    float4 pos = mul(float4(input.posModel, 1.0f), world);
    return mul(pos, viewProj);
}
