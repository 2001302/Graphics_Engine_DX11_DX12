#ifndef _PHYSICALLYBASEDSHADER
#define _PHYSICALLYBASEDSHADER

#include "shader.h"
#include "behavior_tree_builder.h"

using namespace DirectX;

namespace Engine {

struct PhsicallyBasedVertexConstantBuffer {
    Matrix modelWorld;
    Matrix invTranspose;
    Matrix view;
    Matrix projection;
    int useHeightMap = 0;
    float heightScale = 0.0f;
    Vector2 dummy;
};

struct PhsicallyPixelConstantBuffer {
    Vector3 eyeWorld;         // 12
    float mipmapLevel = 0.0f; // 4

    PhysicallyMatrial material; // 48
    Light lights[MAX_LIGHTS]; // 48 * MAX_LIGHTS

    int useAlbedoMap = 1;
    int useNormalMap = 1;
    int useAOMap = 0;         // Ambient Occlusion
    int invertNormalMapY = 0; // 16
    int useMetallicMap = 0;
    int useRoughnessMap = 0;
    int useEmissiveMap = 0;
    float expose = 1.0f; // 16
    float gamma = 1.0f;
    Vector3 dummy; // 16
};

class PhsicallyBasedShader : public IShader {
  public:
    ComPtr<ID3D11SamplerState> clampSamplerState;
};

class PhsicallyBasedShaderSource : public IShaderSource {
  public:
    ComPtr<ID3D11Buffer> vertex_constant_buffer;
    ComPtr<ID3D11Buffer> pixel_constant_buffer;
    PhsicallyBasedVertexConstantBuffer vertex_constant_buffer_data;
    PhsicallyPixelConstantBuffer pixel_constant_buffer_data;

  private:
    void InitializeThis() override {
        vertex_constant_buffer_data.modelWorld = Matrix();
        vertex_constant_buffer_data.view = Matrix();
        vertex_constant_buffer_data.projection = Matrix();
        vertex_constant_buffer_data.useHeightMap = 0;
        vertex_constant_buffer_data.heightScale = 0.0f;

        Direct3D::GetInstance().CreateConstantBuffer(
            vertex_constant_buffer_data,
                                      vertex_constant_buffer);
        Direct3D::GetInstance().CreateConstantBuffer(pixel_constant_buffer_data,
                                      pixel_constant_buffer);
    }
};

class CheckPhysicallyBasedShader : public ConditionalNode {
    EnumBehaviorTreeStatus CheckCondition() override;
};

class InitializePhysicallyBasedShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class UpdateGameObjectsUsingPhysicallyBasedShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class RenderGameObjectsUsingPhysicallyBasedShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

} // namespace Engine
#endif