#ifndef _APPLICATION
#define _APPLICATION

#include "behavior_tree.h"
#include "constant_buffers.h"
#include "env.h"
#include "geometry_generator.h"
#include "graphics_pso.h"
#include "mesh.h"
#include "message.h"
#include "message_receiver.h"
#include "model.h"
#include "pipeline_manager.h"
#include "platform.h"
#include "post_process.h"
#include "setting_ui.h"

namespace platform {

using DirectX::BoundingSphere;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Quaternion;
using DirectX::SimpleMath::Vector3;
using Microsoft::WRL::ComPtr;

class Application : public Platform {
  public:
    Application();

    bool OnStart() override final;
    bool OnFrame() override final;
    bool OnStop() override final;
    LRESULT CALLBACK MessageHandler(HWND main_window, UINT umsg, WPARAM wparam,
                                    LPARAM lparam) override final;

  private:
    std::shared_ptr<dx11::Input> input_;
    std::shared_ptr<common::SettingUi> imgui_;
    std::shared_ptr<dx11::PipelineManager> manager_;
    std::unique_ptr<dx11::MessageReceiver> message_receiver_;

    std::shared_ptr<dx11::Model> m_ground;
    std::shared_ptr<dx11::Model> m_mainObj;
    std::shared_ptr<dx11::Model> m_lightSphere[MAX_LIGHTS];
    std::shared_ptr<dx11::Model> m_skybox;
    std::shared_ptr<dx11::Model> m_cursorSphere;
    std::shared_ptr<dx11::Model> m_screenSquare;

    BoundingSphere m_mainBoundingSphere;

    bool m_usePerspectiveProjection = true;

    // 거울
    std::shared_ptr<dx11::Model> m_mirror;
    DirectX::SimpleMath::Plane m_mirrorPlane;
    float m_mirrorAlpha = 1.0f; // Opacity

    std::vector<std::shared_ptr<dx11::Model>> m_basicList;

    // todo

    // 렌더링 -> PostEffects -> PostProcess
    dx11::PostEffectsConstants m_postEffectsConstsCPU;
    ComPtr<ID3D11Buffer> m_postEffectsConstsGPU;

    dx11::PostProcess m_postProcess;

    // 다양한 Pass들을 더 간단히 구현하기 위해 ConstBuffer들 분리
    dx11::GlobalConstants m_globalConstsCPU;
    dx11::GlobalConstants m_reflectGlobalConstsCPU;
    dx11::GlobalConstants m_shadowGlobalConstsCPU[MAX_LIGHTS];
    ComPtr<ID3D11Buffer> m_globalConstsGPU;
    ComPtr<ID3D11Buffer> m_reflectGlobalConstsGPU;
    ComPtr<ID3D11Buffer> m_shadowGlobalConstsGPU[MAX_LIGHTS];

    // 공통으로 사용하는 텍스춰들
    ComPtr<ID3D11ShaderResourceView> m_envSRV;
    ComPtr<ID3D11ShaderResourceView> m_irradianceSRV;
    ComPtr<ID3D11ShaderResourceView> m_specularSRV;
    ComPtr<ID3D11ShaderResourceView> m_brdfSRV;

    bool m_lightRotate = false;

    // 제거필요

    void InitCubemaps(std::wstring basePath, std::wstring envFilename,
                      std::wstring specularFilename,
                      std::wstring irradianceFilename,
                      std::wstring brdfFilename);

    void UpdateGlobalConstants(const Vector3 &eyeWorld, const Matrix &viewRow,
                               const Matrix &projRow, const Matrix &refl);

};
} // namespace platform
#endif
