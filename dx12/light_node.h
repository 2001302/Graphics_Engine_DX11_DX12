#ifndef _LIGHTNODE
#define _LIGHTNODE

#include "foundation/behavior_tree_builder.h"
#include "black_board.h"
#include "mesh_renderer.h"

namespace graphics {

class LightNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto target = black_board->targets.get();
        auto condition = black_board->conditions.get();

        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {
            {
                condition->global_consts.GetCpu().lights[0].radiance = Vector3(5.0f);
                condition->global_consts.GetCpu().lights[0].position =
                    Vector3(0.0f, 1.5f, 1.1f);
                condition->global_consts.GetCpu().lights[0].direction =
                    Vector3(0.0f, -1.0f, 0.0f);
                condition->global_consts.GetCpu().lights[0].spotPower = 3.0f;
                condition->global_consts.GetCpu().lights[0].radius = 0.02f;
                condition->global_consts.GetCpu().lights[0].type =
                    LIGHT_SPOT | LIGHT_SHADOW; // Point with shadow

                condition->global_consts.GetCpu().lights[1].radiance = Vector3(5.0f);
                condition->global_consts.GetCpu().lights[1].spotPower = 3.0f;
                condition->global_consts.GetCpu().lights[1].fallOffEnd = 20.0f;
                condition->global_consts.GetCpu().lights[1].radius = 0.02f;
                condition->global_consts.GetCpu().lights[1].type =
                    LIGHT_SPOT | LIGHT_SHADOW; // Point with shadow

                condition->global_consts.GetCpu().lights[2].type = LIGHT_OFF;
            }

            //{
            //    for (int i = 0; i < MAX_LIGHTS; i++) {
            //        MeshData sphere =
            //            GeometryGenerator::MakeSphere(1.0f, 20, 20);

            //        auto renderer =
            //            std::make_shared<MeshRenderer>(std::vector{sphere});

            //        renderer->UpdateWorldRow(Matrix::CreateTranslation(
            //            condition->global_consts.GetCpu().lights[i].position));
            //        renderer->material_consts.GetCpu().albedoFactor =
            //            Vector3(0.0f);
            //        renderer->material_consts.GetCpu().emissionFactor =
            //            Vector3(1.0f, 1.0f, 0.0f);
            //        renderer->cast_shadow =
            //            false; 

            //        if (condition->global_consts.GetCpu().lights[i].type == 0)
            //            renderer->is_visible = false;

            //        light_spheres[i] = std::make_shared<Model>();
            //        light_spheres[i]->AddComponent(EnumComponentType::eRenderer,
            //                                       renderer);
            //    }
            //}

            break;
        }
        case EnumStageType::eUpdate: {

            float dt = condition->dt;

            static Vector3 lightDev = Vector3(1.0f, 0.0f, 0.0f);
            if (condition->light_rotate) {
                lightDev = Vector3::Transform(
                    lightDev, Matrix::CreateRotationY(dt * 3.141592f * 0.5f));
            }
            condition->global_consts.GetCpu().lights[1].position =
                Vector3(0.0f, 1.1f, 2.0f) + lightDev;
            Vector3 focusPosition = Vector3(0.0f, -0.5f, 1.7f);
            condition->global_consts.GetCpu().lights[1].direction =
                focusPosition - condition->global_consts.GetCpu().lights[1].position;
            condition->global_consts.GetCpu().lights[1].direction.Normalize();

            // for (int i = 0; i < MAX_LIGHTS; i++) {
            //     auto renderer = (MeshRenderer
            //     *)light_spheres[i]->GetComponent(
            //         EnumComponentType::eRenderer);

            //    renderer->UpdateWorldRow(
            //        Matrix::CreateScale((std::max)(
            //            0.01f,
            //            condition->global_consts.GetCpu().lights[i].radius)) *
            //        Matrix::CreateTranslation(
            //            condition->global_consts.GetCpu().lights[i].position));
            //}

            // for (auto &i : light_spheres) {
            //     auto renderer = (MeshRenderer *)i->GetComponent(
            //         EnumComponentType::eRenderer);
            //     renderer->UpdateConstantBuffers();
            // }
            break;
        }
        case EnumStageType::eRender: {

            // dx11::Util::SetPipelineState(
            //     condition->draw_wire ? dx11::pso::defaultWirePSO
            //                            : dx11::pso::defaultSolidPSO);
            // dx11::Util::SetGlobalConsts(condition->global_consts_GPU);
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

        return common::EnumBehaviorTreeStatus::eSuccess;
    }

    //std::shared_ptr<Model> light_spheres[MAX_LIGHTS];
};

} // namespace core

#endif
