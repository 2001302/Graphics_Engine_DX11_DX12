#ifndef _MIRROR_EFFECT_NODE
#define _MIRROR_EFFECT_NODE

#include "../pipeline/black_board.h"
#include "../gpu/pso/mesh_pso.h"
#include "../component/mirror_renderer.h"
#include "../gpu/pso/skybox_pso.h"
#include "../component/skybox_renderer.h"
#include "../gpu/pso/stencil_mark_pso.h"
#include <behaviortree/behavior_tree_builder.h>

namespace graphics {

class MirrorObjectNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto condition = black_board->conditions.get();
        auto targets = black_board->targets.get();

        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {

            // pipeline state object
            stencil_mark_PSO = std::make_shared<StencilMarkPSO>();
            stencil_mark_PSO->Initialize();

            reflect_mesh_solid_PSO = std::make_shared<ReflectSolidMeshPSO>();
            reflect_mesh_solid_PSO->Initialize();

            skinned_reflect_mesh_solid_PSO =
                std::make_shared<ReflectSkinnedSolidMeshPSO>();
            skinned_reflect_mesh_solid_PSO->Initialize();

            reflect_skybox_solid_PSO =
                std::make_shared<SolidReflectSkyboxPSO>();
            reflect_skybox_solid_PSO->Initialize();

            mirror_blend_solid_PSO =
                std::make_shared<MirrorBlendSolidMeshPSO>();
            mirror_blend_solid_PSO->Initialize();

            mirror_mesh_solid_PSO = std::make_shared<SolidMeshPSO>();
            mirror_mesh_solid_PSO->Initialize();

            // global const
            reflect_global_consts.Initialize();
            break;
        }
        case EnumStageType::eUpdate: {
            MirrorRenderer *mirror = nullptr;
            if (targets->world->TryGet(mirror)) {

                const Matrix reflectRow =
                    Matrix::CreateReflection(mirror->GetMirrorPlane());

                reflect_global_consts.CopyCpu(
                    condition->global_consts.GetCpu());

                reflect_global_consts.GetCpu().view =
                    (reflectRow * targets->camera->GetView()).Transpose();
                reflect_global_consts.GetCpu().viewProj =
                    (reflectRow * targets->camera->GetView() *
                     targets->camera->GetProjection())
                        .Transpose();
                reflect_global_consts.GetCpu().invViewProj =
                    reflect_global_consts.GetCpu().viewProj.Invert();

                reflect_global_consts.Upload();

                mirror->UpdateConstantBuffers();
            }
            break;
        }
        case EnumStageType::eRender: {

            MirrorRenderer *ground = nullptr;
            if (targets->world->TryGet(ground)) {

                // on mirror
                if (ground->GetMirrorAlpha() < 1.0f && condition->use_mirror) {

                    // 1.stencilMaskPSO
                    stencil_mark_PSO->Render(
                        targets->world.get(), condition->shared_sampler,
                        condition->global_consts.Get(), ground);

                    // 2.reflectSolidPSO
                    {
                        for (auto &i : targets->objects) {
                            MeshRenderer *renderer = nullptr;

                            if (i.second->TryGet(renderer)) {

                                Animator *animator = nullptr;
                                if (i.second->TryGet(animator)) {
                                    skinned_reflect_mesh_solid_PSO->Render(
                                        targets->world.get(),
                                        condition->shared_sampler,
                                        reflect_global_consts.Get(), renderer,
                                        animator);
                                } else {
                                    reflect_mesh_solid_PSO->Render(
                                        targets->world.get(),
                                        condition->shared_sampler,
                                        reflect_global_consts.Get(), renderer);
                                }
                            }
                        }
                    }

                    // 3.reflectSkyboxSolidPSO
                    {
                        SkyboxRenderer *renderer = nullptr;
                        if (targets->world->TryGet(renderer) &&
                            !renderer->IsBlack()) {
                            reflect_skybox_solid_PSO->Render(
                                condition->shared_sampler, targets->world.get(),
                                reflect_global_consts.Get(), renderer);
                        }
                    }

                    // 4.mirrorBlendSolidPSO
                    mirror_blend_solid_PSO->Render(
                        targets->world.get(), condition->shared_sampler,
                        condition->global_consts.Get(), ground);
                } else {
                    mirror_mesh_solid_PSO->Render(
                        targets->world.get(), condition->shared_sampler,
                        condition->global_consts.Get(), ground);
                }
            }

            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }

    ConstantBuffer<GlobalConstants> reflect_global_consts;

    std::shared_ptr<StencilMarkPSO> stencil_mark_PSO;
    std::shared_ptr<ReflectSolidMeshPSO> reflect_mesh_solid_PSO;
    std::shared_ptr<ReflectSkinnedSolidMeshPSO> skinned_reflect_mesh_solid_PSO;
    std::shared_ptr<SolidReflectSkyboxPSO> reflect_skybox_solid_PSO;
    std::shared_ptr<WireReflectSkyboxPSO> reflect_skybox_wire_PSO;
    std::shared_ptr<MirrorBlendSolidMeshPSO> mirror_blend_solid_PSO;
    std::shared_ptr<SolidMeshPSO> mirror_mesh_solid_PSO;
};
} // namespace graphics

#endif
