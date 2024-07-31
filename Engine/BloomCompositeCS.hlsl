Texture2D<float4> gOriginal : register(t0);
Texture2D<float4> gBloom : register(t1);
RWTexture2D<float4> gOutput : register(u0);

[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float4 original = gOriginal[DTid.xy];
    float4 bloom = gBloom[DTid.xy];
    gOutput[DTid.xy] = original + bloom; // Combine original and bloom images
}