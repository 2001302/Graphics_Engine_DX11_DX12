Texture2D<float4> gInput : register(t0);
RWTexture2D<float4> gOutput : register(u0);

[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float4 color = gInput[DTid.xy];
    float luminance = dot(color.rgb, float3(0.2126, 0.7152, 0.0722)); // Calculate luminance
    if (luminance > 0.8) // Threshold for brightness
    {
        gOutput[DTid.xy] = color;
    }
    else
    {
        gOutput[DTid.xy] = float4(0, 0, 0, 0);
    }
}