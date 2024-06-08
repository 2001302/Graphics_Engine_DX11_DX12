// 쉐이더에서 include할 내용들은 .hlsli 파일에 작성
// Properties -> Item Type: Does not participate in build으로 설정

/* 참고: C++ SimpleMath -> HLSL */
// Vector3 -> float3
// float3 a = normalize(b);
// float a = dot(v1, v2);
// Satuarate() -> saturate() 사용
// float l = length(v);
// struct A{ float a = 1.0f; }; <- 구조체 안에서 초기화 불가
// Vector3(0.0f) -> float3(0.0f, 0.0f, 0.0f)
// Vector4::Transform(v, M) -> mul(v, M)

#define MAX_LIGHTS 3 
#define NUM_DIR_LIGHTS 1
#define NUM_POINT_LIGHTS 1
#define NUM_SPOT_LIGHTS 1

struct Material
{
    float3 ambient;
    float shininess;
    float3 diffuse;
    float dummy1; 
    float3 specular;
    float dummy2;
};

struct Light
{
    float3 strength;
    float fallOffStart;
    float3 direction;
    float fallOffEnd;
    float3 position;
    float spotPower;
};

struct VertexShaderInput
{
    float3 posModel : POSITION; 
    float3 normalModel : NORMAL;   
    float2 texcoord : TEXCOORD0; 
};

struct PixelShaderInput
{
    float4 posProj : SV_POSITION; 
    float3 posWorld : POSITION; 
    float3 normalWorld : NORMAL;
    float2 texcoord : TEXCOORD;
};
