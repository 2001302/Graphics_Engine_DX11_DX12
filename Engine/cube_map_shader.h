#ifndef _CUBEMAPSHADER
#define _CUBEMAPSHADER

#include "shader.h"
#include "behavior_tree_builder.h"

using namespace DirectX;

namespace Engine {

struct CubeMapShader : public IShader {
  public:
    struct CubeMapVertexConstantBuffer {
        Matrix model;
        Matrix invTranspose;
        Matrix view;
        Matrix projection;
    };

    struct CubeMapPixelConstantBuffer {
        int textureToDraw = 0; // 0: Env, 1: Specular, 2: Irradiance
        float mipLevel = 0.0f;
        float dummy1;
        float dummy2;
    };
};
struct CubeMapShaderSource : public IShaderSource {
  public:
    ComPtr<ID3D11Buffer> vertex_constant_buffer;
    ComPtr<ID3D11Buffer> pixel_constant_buffer;
    CubeMapShader::CubeMapVertexConstantBuffer vertex_constant_buffer_data;
    CubeMapShader::CubeMapPixelConstantBuffer pixel_constant_buffer_data;

    ComPtr<ID3D11ShaderResourceView> env_SRV;
    ComPtr<ID3D11ShaderResourceView> specular_SRV;   // Radiance
    ComPtr<ID3D11ShaderResourceView> irradiance_SRV; // Diffuse
    ComPtr<ID3D11ShaderResourceView> brdf_SRV;       // BRDF LookUpTable
  private:
    void InitializeThis() override;
};

class InitializeCubeMapShader : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class UpdateCubeMap : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class RenderCubeMap : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

} // namespace Engine
#endif
