Texture2D<float4> gInput : register(t0);
RWTexture2D<float4> gOutput : register(u0);

cbuffer BlurParams : register(b0)
{
    float2 gTexelSize;
    float gBlurRadius;
};

[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float4 color = float4(0, 0, 0, 0);
    float weightSum = 0;
    for (int i = -int(gBlurRadius); i <= int(gBlurRadius); ++i)
    {
        float weight = exp(-0.5 * (i * i) / (gBlurRadius * gBlurRadius));
        color += gInput[DTid.xy + int2(i, 0)] * weight;
        weightSum += weight;
    }
    gOutput[DTid.xy] = color / weightSum;
}