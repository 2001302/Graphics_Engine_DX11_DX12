#ifndef _SHADOW_EFFECT_NODE
#define _SHADOW_EFFECT_NODE

#include "black_board.h"
#include "mesh_renderer.h"
#include "shadow_mapping_pso.h"
#include <behavior_tree_builder.h>

namespace graphics {

class ShadowEffectNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto condition = black_board->conditions.get();
        auto targets = black_board->targets.get();

        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {

            // pipeline state object
            shadow_map_PSO = std::make_shared<ShadowMappingPSO>();
            shadow_map_PSO->Initialize();

            // global const
            for (int i = 0; i < MAX_LIGHTS; i++) {
                shadow_global_consts[i].Initialize();
            }
            break;
        }
        case EnumStageType::eUpdate: {
            // viewpoint for creating the shadow map
            for (int i = 0; i < MAX_LIGHTS; i++) {
                const auto &light = condition->global_consts.GetCpu().lights[i];
                if (light.type & LIGHT_SHADOW) {

                    Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
                    if (abs(up.Dot(light.direction) + 1.0f) < 1e-5)
                        up = Vector3(1.0f, 0.0f, 0.0f);

                    Matrix lightViewRow = DirectX::XMMatrixLookAtLH(
                        light.position, light.position + light.direction, up);

                    Matrix lightProjRow = DirectX::XMMatrixPerspectiveFovLH(
                        DirectX::XMConvertToRadians(120.0f), 1.0f, 0.1f, 10.0f);

                    shadow_global_consts[i].GetCpu().eyeWorld = light.position;
                    shadow_global_consts[i].GetCpu().view =
                        lightViewRow.Transpose();
                    shadow_global_consts[i].GetCpu().proj =
                        lightProjRow.Transpose();
                    shadow_global_consts[i].GetCpu().invProj =
                        lightProjRow.Invert().Transpose();
                    shadow_global_consts[i].GetCpu().viewProj =
                        (lightViewRow * lightProjRow).Transpose();

                    shadow_global_consts[i].Upload();

                    condition->global_consts.GetCpu().lights[i].viewProj =
                        shadow_global_consts[i].GetCpu().viewProj;
                    condition->global_consts.GetCpu().lights[i].invProj =
                        shadow_global_consts[i].GetCpu().invProj;
                }
            }
            break;
        }
        case EnumStageType::eRender: {

            for (int i = 0; i < MAX_LIGHTS; i++) {
                auto depth_buffer = dynamic_cast<DepthBuffer *>(
                    condition->shadow_texture->GetResource(i));

                shadow_map_PSO->Render(targets->skybox.get(),
                                       condition->shared_sampler,
                                       shadow_global_consts[i].Get(),
                                       targets->objects, depth_buffer);
            }
            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }

    ConstantBuffer<GlobalConstants> shadow_global_consts[MAX_LIGHTS];
    std::shared_ptr<ShadowMappingPSO> shadow_map_PSO;
};
} // namespace graphics

#endif
