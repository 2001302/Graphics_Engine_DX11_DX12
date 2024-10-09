#ifndef _RENDERER_DRAW_NODE
#define _RENDERER_DRAW_NODE

#include "black_board.h"
#include "mesh_renderer.h"
#include "skinned_mesh_renderer.h"
#include <behavior_tree_builder.h>

namespace graphics {

class GameObjectNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->targets;
        auto gui = black_board->gui;

        switch (manager->stage_type) {
        case EnumStageType::eInitialize: {

            // additional object 1
            {
                MeshData mesh = GeometryGenerator::MakeSphere(0.2f, 200, 200);
                Vector3 center(0.5f, 0.5f, 2.0f);

                auto renderer =
                    std::make_shared<MeshRenderer>(std::vector{mesh});

                renderer->UpdateWorldRow(Matrix::CreateTranslation(center));
                renderer->material_consts.GetCpu().albedoFactor =
                    Vector3(0.1f, 0.1f, 1.0f);
                renderer->material_consts.GetCpu().roughnessFactor = 0.2f;
                renderer->material_consts.GetCpu().metallicFactor = 0.6f;
                renderer->material_consts.GetCpu().emissionFactor =
                    Vector3(0.0f);
                renderer->UpdateConstantBuffers();

                auto obj = std::make_shared<common::Model>();
                obj->AddComponent(common::EnumComponentType::eRenderer,
                                  renderer);

                manager->objects.insert({obj->GetEntityId(), obj});
            }

            // additional object 2
            {
                MeshData mesh = GeometryGenerator::MakeBox(0.2f);
                Vector3 center(0.0f, 0.5f, 2.5f);

                auto renderer =
                    std::make_shared<MeshRenderer>(std::vector{mesh});

                renderer->UpdateWorldRow(Matrix::CreateTranslation(center));
                renderer->material_consts.GetCpu().albedoFactor =
                    Vector3(1.0f, 0.2f, 0.2f);
                renderer->material_consts.GetCpu().roughnessFactor = 0.5f;
                renderer->material_consts.GetCpu().metallicFactor = 0.9f;
                renderer->material_consts.GetCpu().emissionFactor =
                    Vector3(0.0f);
                renderer->UpdateConstantBuffers();

                auto obj = std::make_shared<common::Model>();
                obj->AddComponent(common::EnumComponentType::eRenderer,
                                  renderer);

                manager->objects.insert({obj->GetEntityId(), obj});
            }

            break;
        }
        case EnumStageType::eUpdate: {

            for (auto &i : manager->objects) {
                auto renderer = (MeshRenderer *)i.second->GetComponent(
                    common::EnumComponentType::eRenderer);
                renderer->UpdateConstantBuffers();
            }

            break;
        }
        case EnumStageType::eRender: {

            graphics::Util::SetPipelineState(
                manager->draw_wire ? graphics::pipeline::defaultWirePSO
                                   : graphics::pipeline::defaultSolidPSO);
            graphics::Util::SetGlobalConsts(manager->global_consts_GPU);

            for (auto &i : manager->objects) {
                auto renderer = (MeshRenderer *)i.second->GetComponent(
                    common::EnumComponentType::eRenderer);
                renderer->Render();
            }

            // If there is no need to draw mirror reflections, draw only the
            // opaque mirror
            if (manager->ground->mirror_alpha == 1.0f) {
                auto renderer =
                    (MeshRenderer *)manager->ground->mirror->GetComponent(
                        common::EnumComponentType::eRenderer);
                renderer->Render();
            }

            graphics::Util::SetPipelineState(graphics::pipeline::normalsPSO);
            for (auto &i : manager->objects) {
                auto renderer = (MeshRenderer *)i.second->GetComponent(
                    common::EnumComponentType::eRenderer);
                if (renderer->draw_normals)
                    renderer->RenderNormals();
            }

            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};
} // namespace graphics

#endif