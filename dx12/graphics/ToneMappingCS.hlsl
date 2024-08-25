// ToneMappingCS.hlsl
cbuffer Constants : register(b0) { float exposure; }

Texture2D<float4> g_Input : register(t0);
RWTexture2D<uint> g_Output : register(u0);

[numthreads(16, 16, 1)] void main(uint3 DTid
                                  : SV_DispatchThreadID) {
    float4 color = g_Input[DTid.xy];

    // Simple Reinhard tone mapping
    color.rgb = color.rgb / (color.rgb + 1.0);

    // Apply exposure
    color.rgb *= exposure;

    // Convert to uint
    uint4 outputColor = uint4(color * 255.0);

    g_Output[DTid.xy] = outputColor;
}