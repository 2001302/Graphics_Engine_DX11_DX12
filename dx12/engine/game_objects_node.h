#ifndef _RENDERER_DRAW_NODE
#define _RENDERER_DRAW_NODE

#include "../foundation/behavior_tree_builder.h"
#include "../graphics/command_pool.h"
#include "black_board.h"
#include "mesh_renderer.h"

namespace core {

class GameObjectNodeInvoker : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto condition = black_board->conditions.get();
        auto targets = black_board->targets.get();

        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {

            // pso
            mesh_solid_PSO = std::make_shared<dx12::SolidMeshPSO>();
            mesh_solid_PSO->Initialize();

            // sample object
            Vector3 center(0.0f, 0.0f, 0.0f);
            std::string base_path = "Assets/Characters/Mixamo/";
            std::string file_name = "character.fbx";

            auto renderer = std::make_shared<MeshRenderer>();
            renderer->Initialize(base_path, file_name,
                                 condition->command_pool->Get(0));

            renderer->material_consts.GetCpu().albedoFactor = Vector3(1.0f);
            renderer->material_consts.GetCpu().roughnessFactor = 0.8f;
            renderer->material_consts.GetCpu().metallicFactor = 0.0f;
            renderer->UpdateWorldRow(Matrix::CreateTranslation(center));
            renderer->UpdateConstantBuffers();

            auto obj = std::make_shared<Model>();
            obj->AddComponent(EnumComponentType::eRenderer, renderer);

            targets->objects.insert({obj->GetEntityId(), obj});

            break;
        }
        case EnumStageType::eUpdate: {
            for (auto &i : targets->objects) {
                auto renderer = (MeshRenderer *)i.second->GetComponent(
                    EnumComponentType::eRenderer);
                renderer->UpdateConstantBuffers();
            }

            break;
        }
        case EnumStageType::eRender: {
            for (auto &i : targets->objects) {
                auto renderer = (MeshRenderer *)i.second->GetComponent(
                    EnumComponentType::eRenderer);
                renderer->Render(condition, mesh_solid_PSO.get());
            }

            break;
        }
        default:
            break;
        }

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }

    std::shared_ptr<dx12::SolidMeshPSO> mesh_solid_PSO;
    std::shared_ptr<dx12::WireMeshPSO> mesh_wire_PSO;
};
} // namespace core

#endif
