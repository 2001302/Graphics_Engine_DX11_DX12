StructuredBuffer<float4> inputBuffer : register(t0);
RWStructuredBuffer<uint> outputBuffer : register(u0);

cbuffer Constants : register(b0) {
    uint width;
};

float4 ToneMapping(float4 color) {
    color.rgb = color.rgb / (color.rgb + 1.0);
    return color;
}

[numthreads(16, 16, 1)] void main(uint3 DTid
                                    : SV_DispatchThreadID) {
    uint index = DTid.x + DTid.y * width;

    float4 color = inputBuffer[index];

    color = ToneMapping(color);

    uint packedColor = (uint(color.x * 255.0f) & 0xFF) |
                       ((uint(color.y * 255.0f) & 0xFF) << 8) |
                       ((uint(color.z * 255.0f) & 0xFF) << 16) |
                       ((uint(color.w * 255.0f) & 0xFF) << 24);

    outputBuffer[index] = packedColor;
}