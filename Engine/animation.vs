
cbuffer MatrixBuffer {
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 boneTransforms[256];  // 각 본에 대한 변환을 위한 배열
};

struct VertexInputType {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
    uint4 boneIndices : BONEINDICES;
    float4 boneWeights : BONEWEIGHTS;
};

struct PixelInputType {
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

PixelInputType MainVS(VertexInputType input) {

    PixelInputType output;

    // 최종 본 변환을 위한 제로 행렬로 시작
    float4x4 finalBoneTransform = 0.0;
    for (int i = 0; i < 4; ++i) {
        finalBoneTransform += boneTransforms[input.boneIndices[i]] * input.boneWeights[i];
    }

    // 버텍스 위치와 노멀 변환
    float4 worldPosition = mul(float4(input.position, 1.0), finalBoneTransform);
    worldPosition = mul(worldPosition, worldMatrix);
    output.position = mul(worldPosition, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    float4 worldNormal = mul(float4(input.normal, 0.0), finalBoneTransform);
    worldNormal = mul(worldNormal, worldMatrix);
    output.normal = normalize(worldNormal.xyz);

    output.tex = input.tex;

    return output;
}