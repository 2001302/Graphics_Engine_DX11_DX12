#ifndef _LIGHTNODE
#define _LIGHTNODE

#include "black_board.h"
#include "mesh_renderer.h"
#include <behaviortree/behavior_tree_builder.h>

namespace graphics {

class LightNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto job_context = black_board->targets.get();

        switch (job_context->stage_type) {
        case EnumStageType::eInitialize: {
            // 조명 설정
            {
                // 조명 0은 고정
                job_context->global_consts_CPU.lights[0].radiance =
                    Vector3(5.0f);
                job_context->global_consts_CPU.lights[0].position =
                    Vector3(0.0f, 1.5f, 1.1f);
                job_context->global_consts_CPU.lights[0].direction =
                    Vector3(0.0f, -1.0f, 0.0f);
                job_context->global_consts_CPU.lights[0].spotPower = 3.0f;
                job_context->global_consts_CPU.lights[0].radius = 0.02f;
                job_context->global_consts_CPU.lights[0].type =
                    LIGHT_SPOT | LIGHT_SHADOW; // Point with shadow

                // 조명 1의 위치와 방향은 Update()에서 설정
                job_context->global_consts_CPU.lights[1].radiance =
                    Vector3(5.0f);
                job_context->global_consts_CPU.lights[1].spotPower = 3.0f;
                job_context->global_consts_CPU.lights[1].fallOffEnd = 20.0f;
                job_context->global_consts_CPU.lights[1].radius = 0.02f;
                job_context->global_consts_CPU.lights[1].type =
                    LIGHT_SPOT | LIGHT_SHADOW; // Point with shadow

                // 조명 2는 꺼놓음
                job_context->global_consts_CPU.lights[2].type = LIGHT_OFF;
            }

            // 조명 위치 표시
            {
                for (int i = 0; i < MAX_LIGHTS; i++) {
                    MeshData sphere =
                        GeometryGenerator::MakeSphere(1.0f, 20, 20);

                    auto renderer =
                        std::make_shared<MeshRenderer>(std::vector{sphere});

                    renderer->UpdateWorldRow(Matrix::CreateTranslation(
                        job_context->global_consts_CPU.lights[i].position));
                    renderer->material_consts.GetCpu().albedoFactor =
                        Vector3(0.0f);
                    renderer->material_consts.GetCpu().emissionFactor =
                        Vector3(1.0f, 1.0f, 0.0f);
                    renderer->cast_shadow =
                        false; // 조명 표시 물체들은 그림자 X

                    if (job_context->global_consts_CPU.lights[i].type == 0)
                        renderer->is_visible = false;

                    light_spheres[i] = std::make_shared<common::Model>();
                    light_spheres[i]->TryAdd(renderer);
                }
            }

            break;
        }
        case EnumStageType::eUpdate: {

            float dt = job_context->delta_time;

            // 회전하는 lights[1] 업데이트
            static Vector3 lightDev = Vector3(1.0f, 0.0f, 0.0f);
            if (job_context->light_rotate) {
                lightDev = Vector3::Transform(
                    lightDev, Matrix::CreateRotationY(dt * 3.141592f * 0.5f));
            }
            job_context->global_consts_CPU.lights[1].position =
                Vector3(0.0f, 1.1f, 2.0f) + lightDev;
            Vector3 focusPosition = Vector3(0.0f, -0.5f, 1.7f);
            job_context->global_consts_CPU.lights[1].direction =
                focusPosition -
                job_context->global_consts_CPU.lights[1].position;
            job_context->global_consts_CPU.lights[1].direction.Normalize();

            // 조명의 위치 반영
            for (int i = 0; i < MAX_LIGHTS; i++) {
                MeshRenderer *renderer = nullptr;
                if (light_spheres[i]->TryGet(renderer)) {
					renderer->UpdateWorldRow(
						Matrix::CreateScale((std::max)(0.01f,
													  job_context->global_consts_CPU
														  .lights[i]
														  .radius)) *
						Matrix::CreateTranslation(
							job_context->global_consts_CPU.lights[i].position));
				}
            }

            for (auto &i : light_spheres) {
                MeshRenderer *renderer = nullptr;
                if (i->TryGet(renderer)) {
					renderer->UpdateConstantBuffers();
				}
            }
            break;
        }
        case EnumStageType::eRender: {

            graphics::Util::SetPipelineState(
                job_context->draw_wire ? graphics::pipeline::defaultWirePSO
                                       : graphics::pipeline::defaultSolidPSO);
            graphics::Util::SetGlobalConsts(job_context->global_consts_GPU);
            for (auto &i : light_spheres) {
                MeshRenderer *renderer = nullptr;
                if (i->TryGet(renderer)) {
					renderer->Render();
				}
            }
            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }

    std::shared_ptr<common::Model> light_spheres[MAX_LIGHTS];
};

} // namespace graphics

#endif