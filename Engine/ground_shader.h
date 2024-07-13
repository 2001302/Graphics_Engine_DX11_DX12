#ifndef _GROUNDSHADER
#define _GROUNDSHADER

#include "behavior_tree_builder.h"
#include "shader.h"

using namespace DirectX;

namespace dx11 {

struct GroundShader : public IShader {
    struct GroundMaterial {
        Vector3 ambient = Vector3(0.0f);                 // 12
        float shininess = 0.01f;                         // 4
        Vector3 diffuse = Vector3(0.0f);                 // 12
        float dummy1;                                    // 4
        Vector3 specular = Vector3(1.0f);                // 12
        float dummy2;                                    // 4
        Vector3 fresnelR0 = Vector3(1.0f, 0.71f, 0.29f); // Gold
        float dummy3;
    };

    struct GroundVertexConstantBuffer {
        Matrix model;
        Matrix invTranspose;
        Matrix view;
        Matrix projection;
        int useHeightMap = 1;
        float heightScale = 0.0f;
        Vector2 dummy;
    };

    struct GroundPixelConstantBuffer {
    Vector3 eyeWorld;         // 12
    float mipmapLevel = 0.0f; // 4
    GroundMaterial material;  // 48
    Light lights[MAX_LIGHTS]; // 48 * MAX_LIGHTS
    Vector4 indexColor;       // 피킹(Picking)에 사용
    int useTexture = 0;       // 4
    int useNormalMap = 1;     // 4
    int useAOMap = 1;         // 4, Ambient Occlusion
    int reverseNormalMapY = 0; // 4
    };

};

struct GroundShaderSource : public IShaderSource {
  public:
    ComPtr<ID3D11Buffer> vertex_constant_buffer;
    ComPtr<ID3D11Buffer> pixel_constant_buffer;
    GroundShader::GroundVertexConstantBuffer
        vertex_constant_buffer_data;
    GroundShader::GroundPixelConstantBuffer
        pixel_constant_buffer_data;

  private:
    void InitializeThis() override;
};

class InitializeGroundShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class CheckGroundShader : public ConditionalNode {
    EnumBehaviorTreeStatus CheckCondition() override;
};

class UpdateGroundShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class RenderGroundShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};
} // namespace Engine
#endif
