#ifndef _MIRROR_EFFECT
#define _MIRROR_EFFECT

#include "black_board.h"
#include "mesh_renderer.h"
#include "skybox_renderer.h"
#include <behaviortree/behavior_tree_builder.h>

namespace graphics {

class MirrorObjectNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto targets = black_board->targets.get();

        switch (targets->stage_type) {
        case EnumStageType::eInitialize: {

            graphics::Util::CreateConstBuffer(reflect_global_consts_CPU,
                                              reflect_global_consts_GPU);

            break;
        }
        case EnumStageType::eUpdate: {
            // constant buffer
            MirrorRenderer *mirror = nullptr;
            if (targets->ground->TryGet(mirror)) {

                const Matrix reflectRow =
                    Matrix::CreateReflection(mirror->GetMirrorPlane());

                reflect_global_consts_CPU = targets->global_consts_CPU;
                memcpy(&reflect_global_consts_CPU, &targets->global_consts_CPU,
                       sizeof(targets->global_consts_CPU));
                reflect_global_consts_CPU.view =
                    (reflectRow * targets->camera->GetView()).Transpose();
                reflect_global_consts_CPU.viewProj =
                    (reflectRow * targets->camera->GetView() *
                     targets->camera->GetProjection())
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
            if (targets->ground->TryGet(mirror)) {
                if (mirror->GetMirrorAlpha() < 1.0f) {
                    // Mirror 2. Mark only the mirror position as 1 in the
                    // StencilBuffer.
                    graphics::Util::SetPipelineState(
                        graphics::pipeline::stencilMaskPSO);
                    mirror->Render();

                    // Mirror 3. Render the reflected objects at the mirror
                    // position.
                    graphics::Util::SetPipelineState(
                        targets->draw_wire
                            ? graphics::pipeline::reflectWirePSO
                            : graphics::pipeline::reflectSolidPSO);
                    graphics::Util::SetGlobalConsts(reflect_global_consts_GPU);

                    graphics::GpuCore::Instance()
                        .device_context->ClearDepthStencilView(
                            graphics::GpuCore::Instance()
                                .depth_stencil_view.Get(),
                            D3D11_CLEAR_DEPTH, 1.0f, 0);

                    for (auto &i : targets->objects) {
                        MeshRenderer *renderer = nullptr;
                        if (i.second->TryGet(renderer)) {
                            renderer->Render();
                        }
                    }

                    graphics::Util::SetPipelineState(
                        targets->draw_wire
                            ? graphics::pipeline::reflectSkyboxWirePSO
                            : graphics::pipeline::reflectSkyboxSolidPSO);
                    {
                        SkyboxRenderer *renderer = nullptr;
                        if (targets->skybox->TryGet(renderer)) {
                            renderer->Render();
                        }
                    }

                    // Mirror 4. Draw the mirror itself with the 'Blend'
                    // material
                    graphics::Util::SetPipelineState(
                        targets->draw_wire
                            ? graphics::pipeline::mirrorBlendWirePSO
                            : graphics::pipeline::mirrorBlendSolidPSO);
                    graphics::Util::SetGlobalConsts(targets->global_consts_GPU);
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