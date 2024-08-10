#ifndef _MIRROR_EFFECT
#define _MIRROR_EFFECT

#include "behavior_tree_builder.h"
#include "black_board.h"
#include "mesh_renderer.h"

namespace engine {

class MirrorEffectNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->job_context.get();

        switch (manager->stage_type) {
        case EnumStageType::eInitialize: {

            auto mesh = GeometryGenerator::MakeSquare(5.0);

            auto renderer = std::make_shared<MeshRenderer>(
                GraphicsManager::Instance().device,
                GraphicsManager::Instance().device_context, std::vector{mesh});

            // mesh.albedoTextureFilename =
            //     "../Assets/Textures/blender_uv_grid_2k.png";
            renderer->material_consts.GetCpu().albedoFactor = Vector3(0.1f);
            renderer->material_consts.GetCpu().emissionFactor = Vector3(0.0f);
            renderer->material_consts.GetCpu().metallicFactor = 0.5f;
            renderer->material_consts.GetCpu().roughnessFactor = 0.3f;

            Vector3 position = Vector3(0.0f, -0.5f, 2.0f);
            renderer->UpdateWorldRow(Matrix::CreateRotationX(3.141592f * 0.5f) *
                                     Matrix::CreateTranslation(position));

            manager->ground = std::make_shared<Model>();
            manager->ground->AddComponent(EnumComponentType::eRenderer,
                                          renderer);

            manager->mirror_plane =
                DirectX::SimpleMath::Plane(position, Vector3(0.0f, 1.0f, 0.0f));
            manager->mirror = manager->ground; // 바닥에 거울처럼 반사 구현

            // m_basicList.push_back(m_ground); // 거울은 리스트에 등록 X

            GraphicsUtil::CreateConstBuffer(GraphicsManager::Instance().device,
                                            reflect_global_consts_CPU,
                                            reflect_global_consts_GPU);

            break;
        }
        case EnumStageType::eUpdate: {
            // constant buffer
            {
                const Matrix reflectRow =
                    Matrix::CreateReflection(manager->mirror_plane);

                reflect_global_consts_CPU = manager->global_consts_CPU;
                memcpy(&reflect_global_consts_CPU, &manager->global_consts_CPU,
                       sizeof(manager->global_consts_CPU));
                reflect_global_consts_CPU.view =
                    (reflectRow * manager->camera->GetView()).Transpose();
                reflect_global_consts_CPU.viewProj =
                    (reflectRow * manager->camera->GetView() *
                     manager->camera->GetProjection())
                        .Transpose();

                reflect_global_consts_CPU.invViewProj =
                    reflect_global_consts_CPU.viewProj.Invert();

                GraphicsUtil::UpdateBuffer(
                    GraphicsManager::Instance().device_context,
                    reflect_global_consts_CPU, reflect_global_consts_GPU);
            }

            auto renderer = (MeshRenderer *)manager->mirror->GetComponent(
                EnumComponentType::eRenderer);

            renderer->UpdateConstantBuffers(
                GraphicsManager::Instance().device,
                GraphicsManager::Instance().device_context);

            break;
        }
        case EnumStageType::eRender: {

            // on mirror
            if (manager->mirror_alpha < 1.0f) {

                // Mirror 2. Mark only the mirror position as 1 in the
                // StencilBuffer.
                GraphicsManager::Instance().SetPipelineState(
                    Graphics::stencilMaskPSO);

                if (true) {
                    auto renderer =
                        (MeshRenderer *)manager->mirror->GetComponent(
                            EnumComponentType::eRenderer);
                    renderer->Render(
                        GraphicsManager::Instance().device_context);
                }

                // Mirror 3. Render the reflected objects at the mirror
                // position.
                GraphicsManager::Instance().SetPipelineState(
                    manager->draw_wire ? Graphics::reflectWirePSO
                                       : Graphics::reflectSolidPSO);
                GraphicsManager::Instance().SetGlobalConsts(
                    reflect_global_consts_GPU);

                GraphicsManager::Instance()
                    .device_context->ClearDepthStencilView(
                        GraphicsManager::Instance().m_depthStencilView.Get(),
                        D3D11_CLEAR_DEPTH, 1.0f, 0);

                for (auto &i : manager->models) {
                    auto renderer = (MeshRenderer *)i.second->GetComponent(
                        EnumComponentType::eRenderer);
                    renderer->Render(
                        GraphicsManager::Instance().device_context);
                }

                if (true) {
                    GraphicsManager::Instance().SetPipelineState(
                        manager->draw_wire ? Graphics::reflectSkyboxWirePSO
                                           : Graphics::reflectSkyboxSolidPSO);
                    auto renderer =
                        (MeshRenderer *)manager->skybox->GetComponent(
                            EnumComponentType::eRenderer);
                    renderer->Render(
                        GraphicsManager::Instance().device_context);
                }

                if (true) {
                    // Mirror 4. Draw the mirror itself with the 'Blend'
                    // material
                    GraphicsManager::Instance().SetPipelineState(
                        manager->draw_wire ? Graphics::mirrorBlendWirePSO
                                           : Graphics::mirrorBlendSolidPSO);
                    GraphicsManager::Instance().SetGlobalConsts(
                        manager->global_consts_GPU);
                    auto renderer =
                        (MeshRenderer *)manager->mirror->GetComponent(
                            EnumComponentType::eRenderer);
                    renderer->Render(
                        GraphicsManager::Instance().device_context);
                }

            } // end of if (m_mirrorAlpha < 1.0f)
            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }

    GlobalConstants reflect_global_consts_CPU;
    ComPtr<ID3D11Buffer> reflect_global_consts_GPU;
};

} // namespace engine

#endif