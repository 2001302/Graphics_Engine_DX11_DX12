#ifndef _MIRROR_EFFECT_NODE
#define _MIRROR_EFFECT_NODE

#include "black_board.h"
#include "mesh_pso.h"
#include "mesh_renderer.h"
#include "skybox_pso.h"
#include "stencil_mark_pso.h"
#include <behavior_tree_builder.h>

namespace graphics {

class MirrorEffectNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto condition = black_board->conditions.get();
        auto targets = black_board->targets.get();

        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {

            // pso
            stencil_mark_PSO = std::make_shared<StencilMarkPSO>();
            stencil_mark_PSO->Initialize();

            reflect_mesh_solid_PSO = std::make_shared<ReflectSolidMeshPSO>();
            reflect_mesh_solid_PSO->Initialize();

            reflect_skybox_solid_PSO =
                std::make_shared<SolidReflectSkyboxPSO>();
            reflect_skybox_solid_PSO->Initialize();

            mirror_blend_solid_PSO =
                std::make_shared<MirrorBlendSolidMeshPSO>();
            mirror_blend_solid_PSO->Initialize();

            // global const
            reflect_global_consts.Initialize();

            // ground
            auto mesh = GeometryGenerator::MakeSquare(5.0);

            auto component = std::make_shared<MeshRenderer>();
            component->Initialize(std::vector{mesh});
            component->MaterialConsts().GetCpu().albedo_factor = Vector3(0.1f);
            component->MaterialConsts().GetCpu().emission_factor =
                Vector3(0.0f);
            component->MaterialConsts().GetCpu().metallic_factor = 0.5f;
            component->MaterialConsts().GetCpu().roughness_factor = 0.3f;

            Vector3 position = Vector3(0.0f, -0.5f, 2.0f);
            component->UpdateWorldRow(
                Matrix::CreateRotationX(PI * 0.5f) *
                Matrix::CreateTranslation(position));

            targets->ground = std::make_shared<ReflectableModel>();
            targets->ground->mirror = std::make_shared<Model>();
            targets->ground->mirror->AddComponent(EnumComponentType::eRenderer,
                                                 component);

            targets->ground->mirror_plane =
                DirectX::SimpleMath::Plane(position, Vector3(0.0f, 1.0f, 0.0f));

            targets->objects.insert({targets->ground->mirror->GetEntityId(),
									 targets->ground->mirror});
            break;
        }
        case EnumStageType::eUpdate: {
            const Matrix reflectRow =
                Matrix::CreateReflection(targets->ground->mirror_plane);

            reflect_global_consts.CopyCpu(condition->global_consts.GetCpu());

            reflect_global_consts.GetCpu().view =
                (reflectRow * targets->camera->GetView()).Transpose();
            reflect_global_consts.GetCpu().viewProj =
                (reflectRow * targets->camera->GetView() *
                 targets->camera->GetProjection())
                    .Transpose();
            reflect_global_consts.GetCpu().invViewProj =
                reflect_global_consts.GetCpu().viewProj.Invert();

            reflect_global_consts.Upload();
            break;
        }
        case EnumStageType::eRender: {

            // on mirror
            if (targets->ground->mirror_alpha < 1.0f) {

                // 1.stencilMaskPSO
                {
                    auto component =
                        (MeshRenderer *)targets->ground->mirror->GetComponent(
                            EnumComponentType::eRenderer);
                     stencil_mark_PSO->Render(
                     condition->shared_texture, condition->shared_sampler,
                     condition->global_consts.Get(), component);
                }

                // 2.reflectSolidPSO
                {
                    for (auto &i : targets->objects) {
                        auto component = (MeshRenderer *)i.second->GetComponent(
                            EnumComponentType::eRenderer);

                        reflect_mesh_solid_PSO->Render(
                            condition->shared_texture,
                            condition->shared_sampler,
                            condition->global_consts.Get(), component);
                    }
                }

                // 3.reflectSkyboxSolidPSO
                {
                    auto component =
                        (MeshRenderer *)targets->skybox->GetComponent(
                            EnumComponentType::eRenderer);

                    reflect_skybox_solid_PSO->Render(
                        condition->shared_sampler, condition->shared_texture,
                        condition->global_consts.Get(), component);
                }

                // 4.mirrorBlendSolidPSO
                {
                    auto component =
                        (MeshRenderer *)targets->ground->mirror->GetComponent(
                            EnumComponentType::eRenderer);
                     mirror_blend_solid_PSO->Render(
                     condition->shared_texture, condition->shared_sampler,
                     condition->global_consts.Get(), component);
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
    std::shared_ptr<WireReflectMeshPSO> reflect_mesh_wire_PSO;
    std::shared_ptr<SolidReflectSkyboxPSO> reflect_skybox_solid_PSO;
    std::shared_ptr<WireReflectSkyboxPSO> reflect_skybox_wire_PSO;
    std::shared_ptr<MirrorBlendSolidMeshPSO> mirror_blend_solid_PSO;
    std::shared_ptr<WireMirrorBlendMeshPSO> mirror_blend_wire_PSO;
};
} // namespace graphics

#endif
