#ifndef _IMAGEBASEDSHADER
#define _IMAGEBASEDSHADER

#include "shader.h"
#include "behavior_tree_builder.h"

using namespace DirectX;

namespace Engine {

struct ImageBasedVertexConstantBuffer {
    Matrix model;
    Matrix invTranspose;
    Matrix view;
    Matrix projection;
};

struct ImageBasedPixelConstantBuffer {
    Vector3 eyeWorld;         // 12
    bool useTexture;          // 4
    Material material;        // 48
};

class ImageBasedShader : public IShader {
  public:
};

class ImageBasedShaderSource : public IShaderSource {
  public:
    ComPtr<ID3D11Buffer> vertex_constant_buffer;
    ComPtr<ID3D11Buffer> pixel_constant_buffer;
    ImageBasedVertexConstantBuffer vertex_constant_buffer_data;
    ImageBasedPixelConstantBuffer pixel_constant_buffer_data;

  private:
    void InitializeThis() override {
        vertex_constant_buffer_data.model = Matrix();
        vertex_constant_buffer_data.view = Matrix();
        vertex_constant_buffer_data.projection = Matrix();

        Direct3D::GetInstance().CreateConstantBuffer(
            vertex_constant_buffer_data,
                                      vertex_constant_buffer);
        Direct3D::GetInstance().CreateConstantBuffer(pixel_constant_buffer_data,
                                      pixel_constant_buffer);
    }
};

class CheckImageBasedShader : public ConditionalNode {
    EnumBehaviorTreeStatus CheckCondition() override;
};

class InitializeImageBasedShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class UpdateGameObjectsUsingImageBasedShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class RenderGameObjectsUsingImageBasedShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

} // namespace Engine
#endif