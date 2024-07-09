#ifndef _PHONGSHADER
#define _PHONGSHADER

#include "shader.h"
#include "behavior_tree_builder.h"

using namespace DirectX;

namespace Engine {

/// <summary>
/// 공유되어 사용되는 Phong Shader
/// </summary>
struct PhongShader : public IShader {
    struct VertexConstantBuffer {
        Matrix model;
        Matrix invTranspose;
        Matrix view;
        Matrix projection;
    };

    struct PixelConstantBuffer {
        Vector3 eyeWorld;         // 12
        bool useTexture;          // 4
        Material material;        // 48
        Light lights[MAX_LIGHTS]; // 48 * MAX_LIGHTS
        bool useBlinnPhong = true;
        Vector3 dummy;
    };
};

/// <summary>
/// Phong Shader에서 사용하는 공유되지 않는 Constant Buffer 정보
/// </summary>
struct PhongShaderSource : public IShaderSource {
  public:
    ComPtr<ID3D11Buffer> vertex_constant_buffer;
    ComPtr<ID3D11Buffer> pixel_constant_buffer;
    PhongShader::VertexConstantBuffer vertex_constant_buffer_data;
    PhongShader::PixelConstantBuffer pixel_constant_buffer_data;

  private:
    void InitializeThis() override;
};

class CheckPhongShader : public ConditionalNode {
    EnumBehaviorTreeStatus CheckCondition() override;
};

class InitializePhongShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class UpdateGameObjectsUsingPhongShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class RenderGameObjectsUsingPhongShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

} // namespace Engine
#endif
