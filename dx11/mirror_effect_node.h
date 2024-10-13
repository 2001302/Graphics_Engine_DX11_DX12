#ifndef _MIRROR_EFFECT
#define _MIRROR_EFFECT

#include "black_board.h"
#include "mesh_renderer.h"
#include <behavior_tree_builder.h>

namespace graphics {

class MirrorEffectNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->targets.get();

        switch (manager->stage_type) {
        case EnumStageType::eInitialize: {

            auto mesh = GeometryGenerator::MakeSquare(5.0);

            auto mirror = std::make_shared<MirrorRenderer>(std::vector{mesh});

            // mesh.albedoTextureFilename =
            //     "../Assets/Textures/blender_uv_grid_2k.png";
            mirror->material_consts.GetCpu().albedoFactor = Vector3(0.1f);
            mirror->material_consts.GetCpu().emissionFactor = Vector3(0.0f);
            mirror->material_consts.GetCpu().metallicFactor = 0.5f;
            mirror->material_consts.GetCpu().roughnessFactor = 0.3f;

            Vector3 position = Vector3(0.0f, -0.5f, 2.0f);
            mirror->UpdateWorldRow(Matrix::CreateRotationX(3.141592f * 0.5f) *
                                   Matrix::CreateTranslation(position));

            mirror->SetMirrorPlane(DirectX::SimpleMath::Plane(
                position, Vector3(0.0f, 1.0f, 0.0f)));

            manager->ground = std::make_shared<common::Model>();
            manager->ground->TryAdd(mirror);

            graphics::Util::CreateConstBuffer(reflect_global_consts_CPU,
                                              reflect_global_consts_GPU);

            break;
        }
        case EnumStageType::eUpdate: {
            // constant buffer
            MirrorRenderer *mirror = nullptr;
            if (manager->ground->TryGet(mirror)) {

                const Matrix reflectRow =
                    Matrix::CreateReflection(mirror->GetMirrorPlane());

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

                graphics::Util::UpdateBuffer(reflect_global_consts_CPU,
                                             reflect_global_consts_GPU);

                mirror->UpdateConstantBuffers();
            }

            break;
        }
        case EnumStageType::eRender: {

            // on mirror
            MirrorRenderer *mirror = nullptr;
            if (manager->ground->TryGet(mirror)) {
                if (mirror->GetMirrorAlpha() < 1.0f) {
                    // Mirror 2. Mark only the mirror position as 1 in the
                    // StencilBuffer.
                    graphics::Util::SetPipelineState(
                        graphics::pipeline::stencilMaskPSO);
                    mirror->Render();

                    // Mirror 3. Render the reflected objects at the mirror
                    // position.
                    graphics::Util::SetPipelineState(
                        manager->draw_wire
                            ? graphics::pipeline::reflectWirePSO
                            : graphics::pipeline::reflectSolidPSO);
                    graphics::Util::SetGlobalConsts(reflect_global_consts_GPU);

                    graphics::GpuCore::Instance()
                        .device_context->ClearDepthStencilView(
                            graphics::GpuCore::Instance()
                                .depth_stencil_view.Get(),
                            D3D11_CLEAR_DEPTH, 1.0f, 0);

                    for (auto &i : manager->objects) {
                        MeshRenderer *renderer = nullptr;
                        if (i.second->TryGet(renderer)) {
                            renderer->Render();
                        }
                    }

                    graphics::Util::SetPipelineState(
                        manager->draw_wire
                            ? graphics::pipeline::reflectSkyboxWirePSO
                            : graphics::pipeline::reflectSkyboxSolidPSO);
                    {
                        MeshRenderer *renderer = nullptr;
                        if (manager->skybox->model->TryGet(renderer)) {
                            renderer->Render();
                        }
                    }

                    // Mirror 4. Draw the mirror itself with the 'Blend'
                    // material
                    graphics::Util::SetPipelineState(
                        manager->draw_wire
                            ? graphics::pipeline::mirrorBlendWirePSO
                            : graphics::pipeline::mirrorBlendSolidPSO);
                    graphics::Util::SetGlobalConsts(manager->global_consts_GPU);
                    mirror->Render();
                }
            }

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
}; // namespace graphics
#endif