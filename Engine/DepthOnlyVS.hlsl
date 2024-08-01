#include "Common.hlsli" // 쉐이더에서도 include 사용 가능

float4 main(VertexShaderInput input) : SV_POSITION {
    float4 pos = mul(float4(input.posModel, 1.0f), world);
    return mul(pos, viewProj);
}
