#ifndef _PIPELINEMANAGER
#define _PIPELINEMANAGER

#include "board_map.h"
#include "camera.h"
#include "dataBlock.h"
#include "graphics_manager.h"
#include "model.h"
#include "constant_buffer.h"

namespace engine {
/// <summary>
/// Rendering에 필요한 shared object의 집합.
/// </summary>
class PipelineManager : public common::IDataBlock {
  public:
    std::unique_ptr<Camera> camera;
    //std::unique_ptr<Light> light;
    std::unique_ptr<BoardMap> board_map;
    std::shared_ptr<common::INodeUi> skybox;
    std::shared_ptr<common::INodeUi> ground;
    std::map<int /*id*/, common::INodeUi*> models;
    std::unordered_map<EnumShaderType, std::shared_ptr<IShader>> shaders;

    //shared resource
    // 다양한 Pass들을 더 간단히 구현하기 위해 ConstBuffer들 분리
    GlobalConstants m_globalConstsCPU;
    GlobalConstants m_reflectGlobalConstsCPU;
    GlobalConstants m_shadowGlobalConstsCPU[MAX_LIGHTS];
    ComPtr<ID3D11Buffer> m_globalConstsGPU;
    ComPtr<ID3D11Buffer> m_reflectGlobalConstsGPU;
    ComPtr<ID3D11Buffer> m_shadowGlobalConstsGPU[MAX_LIGHTS];

    // 공통으로 사용하는 텍스춰들
    ComPtr<ID3D11ShaderResourceView> m_envSRV;
    ComPtr<ID3D11ShaderResourceView> m_irradianceSRV;
    ComPtr<ID3D11ShaderResourceView> m_specularSRV;
    ComPtr<ID3D11ShaderResourceView> m_brdfSRV;

};
} // namespace dx11
#endif
