Texture2D<float4> gInput : register(t0);
RWTexture2D<float4> gOutput : register(u0);
cbuffer ImageFilterConstantData : register(b0)
{
    float dx;
    float dy;
    float threshold;
    float strength;
    float4 options;
};

[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float4 color = float4(0, 0, 0, 0);
    float weightSum = 0;
    for (int i = -int(strength); i <= int(strength); ++i)
    {
        float weight = exp(-0.5 * (i * i) / (strength * strength));
        color += gInput[DTid.xy + int2(0, i)] * weight;
        weightSum += weight;
    }
    gOutput[DTid.xy] = color / weightSum;
}