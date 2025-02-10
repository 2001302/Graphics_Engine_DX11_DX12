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

        auto target = black_board->targets.get();
        auto condition = black_board->conditions.get();

        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {
            {
                condition->global_consts.GetCpu().lights[0].radiance =
                    Vector3(5.0f);
                condition->global_consts.GetCpu().lights[0].position =
                    Vector3(0.0f, 1.5f, 1.1f);
                condition->global_consts.GetCpu().lights[0].direction =
                    Vector3(0.0f, -1.0f, 0.0f);
                condition->global_consts.GetCpu().lights[0].spotPower = 3.0f;
                condition->global_consts.GetCpu().lights[0].radius = 0.02f;
                condition->global_consts.GetCpu().lights[0].type =
                    LIGHT_SPOT | LIGHT_SHADOW; // Point with shadow

                condition->global_consts.GetCpu().lights[1].type = LIGHT_OFF;
                condition->global_consts.GetCpu().lights[2].type = LIGHT_OFF;
            }

            break;
        }
        case EnumStageType::eUpdate: {

            float dt = condition->delta_time;

            static Vector3 lightDev = Vector3(1.0f, 0.0f, 0.0f);
            if (condition->light_rotate) {
                lightDev = Vector3::Transform(
                    lightDev, Matrix::CreateRotationY(dt * 3.141592f * 0.5f));
            }
            condition->global_consts.GetCpu().lights[1].position =
                Vector3(0.0f, 1.1f, 2.0f) + lightDev;
            Vector3 focusPosition = Vector3(0.0f, -0.5f, 1.7f);
            condition->global_consts.GetCpu().lights[1].direction =
                focusPosition -
                condition->global_consts.GetCpu().lights[1].position;
            condition->global_consts.GetCpu().lights[1].direction.Normalize();

            break;
        }
        case EnumStageType::eRender: {

            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }

    // std::shared_ptr<Model> light_spheres[MAX_LIGHTS];
};

} // namespace graphics

#endif
