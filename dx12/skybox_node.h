#ifndef _SKYBOX_NODE
#define _SKYBOX_NODE

#include "black_board.h"
#include "mesh_renderer.h"
#include "skybox_pso.h"
#include <behavior_tree_builder.h>

namespace graphics {

class SkyBoxNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto condition = black_board->conditions.get();
        auto targets = black_board->targets.get();

        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {

            skyboxPSO = std::make_shared<SkyboxPSO>();
            skyboxPSO->Initialize();

            auto mesh_data = GeometryGenerator::MakeBox(40.0f);
            std::reverse(mesh_data.indices.begin(), mesh_data.indices.end());

            auto component = std::make_shared<MeshRenderer>();
            component->Initialize(std::vector{mesh_data});

            skybox = std::make_shared<Model>();
            skybox->AddComponent(EnumComponentType::eRenderer, component);

            break;
        }
        case EnumStageType::eRender: {

            auto component = (MeshRenderer *)skybox->GetComponent(
                EnumComponentType::eRenderer);

            skyboxPSO->Render(condition->shared_sampler,
                              condition->shared_texture,
                              condition->global_consts.Get(), component);
            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }

    std::shared_ptr<SkyboxPSO> skyboxPSO;
    std::shared_ptr<Model> skybox;
};
} // namespace graphics

#endif
