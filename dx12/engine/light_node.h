#ifndef _LIGHTNODE
#define _LIGHTNODE

#include "../foundation/behavior_tree_builder.h"
#include "black_board.h"
#include "mesh_renderer.h"

namespace core {

class LightNodeInvoker : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto job_context = black_board->job_context.get();

        switch (job_context->stage_type) {
        case EnumStageType::eInitialize: {
            // 議곕챸 ?ㅼ젙
            {
                // 議곕챸 0? 怨좎젙
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

                // 議곕챸 1???꾩튂? 諛⑺뼢? Update()?먯꽌 ?ㅼ젙
                job_context->global_consts_CPU.lights[1].radiance =
                    Vector3(5.0f);
                job_context->global_consts_CPU.lights[1].spotPower = 3.0f;
                job_context->global_consts_CPU.lights[1].fallOffEnd = 20.0f;
                job_context->global_consts_CPU.lights[1].radius = 0.02f;
                job_context->global_consts_CPU.lights[1].type =
                    LIGHT_SPOT | LIGHT_SHADOW; // Point with shadow

                // 議곕챸 2??爰쇰넃??
                job_context->global_consts_CPU.lights[2].type = LIGHT_OFF;
            }

            //// 議곕챸 ?꾩튂 ?쒖떆
            //{
            //    for (int i = 0; i < MAX_LIGHTS; i++) {
            //        MeshData sphere =
            //            GeometryGenerator::MakeSphere(1.0f, 20, 20);

            //        auto renderer =
            //            std::make_shared<MeshRenderer>(std::vector{sphere});

            //        renderer->UpdateWorldRow(Matrix::CreateTranslation(
            //            job_context->global_consts_CPU.lights[i].position));
            //        renderer->material_consts.GetCpu().albedoFactor =
            //            Vector3(0.0f);
            //        renderer->material_consts.GetCpu().emissionFactor =
            //            Vector3(1.0f, 1.0f, 0.0f);
            //        renderer->cast_shadow =
            //            false; // 議곕챸 ?쒖떆 臾쇱껜?ㅼ? 洹몃┝??X

            //        if (job_context->global_consts_CPU.lights[i].type == 0)
            //            renderer->is_visible = false;

            //        light_spheres[i] = std::make_shared<Model>();
            //        light_spheres[i]->AddComponent(EnumComponentType::eRenderer,
            //                                       renderer);
            //    }
            //}

            break;
        }
        case EnumStageType::eUpdate: {

            float dt = job_context->dt;

            // ?뚯쟾?섎뒗 lights[1] ?낅뜲?댄듃
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

            //// 議곕챸???꾩튂 諛섏쁺
            //for (int i = 0; i < MAX_LIGHTS; i++) {
            //    auto renderer = (MeshRenderer *)light_spheres[i]->GetComponent(
            //        EnumComponentType::eRenderer);

            //    renderer->UpdateWorldRow(
            //        Matrix::CreateScale((std::max)(
            //            0.01f,
            //            job_context->global_consts_CPU.lights[i].radius)) *
            //        Matrix::CreateTranslation(
            //            job_context->global_consts_CPU.lights[i].position));
            //}

            //for (auto &i : light_spheres) {
            //    auto renderer = (MeshRenderer *)i->GetComponent(
            //        EnumComponentType::eRenderer);
            //    renderer->UpdateConstantBuffers();
            //}
            break;
        }
        case EnumStageType::eRender: {

            //dx11::Util::SetPipelineState(
            //    job_context->draw_wire ? dx11::pso::defaultWirePSO
            //                           : dx11::pso::defaultSolidPSO);
            //dx11::Util::SetGlobalConsts(job_context->global_consts_GPU);
            //for (auto &i : light_spheres) {
            //    auto renderer = (MeshRenderer *)i->GetComponent(
            //        EnumComponentType::eRenderer);
            //    renderer->Render();
            //}
            break;
        }
        default:
            break;
        }

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }

    //std::shared_ptr<Model> light_spheres[MAX_LIGHTS];
};

} // namespace core

#endif
