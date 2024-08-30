#ifndef _SKYBOX_NODE
#define _SKYBOX_NODE

#include "../foundation/behavior_tree_builder.h"
#include "../graphics/command_pool.h"
#include "../graphics/skybox_pso.h"
#include "black_board.h"
#include "mesh_renderer.h"

namespace core {

class SkyBoxNodeInvoker : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto condition = black_board->conditions.get();
        auto targets = black_board->targets.get();

        auto command_list = condition->command_pool->Get(0);
        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {

            auto mesh_data = GeometryGenerator::MakeBox(40.0f);
            std::reverse(mesh_data.indices.begin(), mesh_data.indices.end());

            auto renderer =
                std::make_shared<MeshRenderer>(std::vector{mesh_data});
            renderer->Initialize(std::vector{mesh_data}, command_list);

            skybox = std::make_shared<Model>();
            skybox->AddComponent(EnumComponentType::eRenderer, renderer);

            auto env_name = L"./Assets/Textures/Cubemaps/HDRI/SampleEnvHDR.dds";
            auto specular_name =
                L"./Assets/Textures/Cubemaps/HDRI/SampleSpecularHDR.dds";
            auto irradiance_name =
                L"./Assets/Textures/Cubemaps/HDRI/SampleDiffuseHDR.dds";
            auto brdf_name = L"./Assets/Textures/Cubemaps/HDRI/SampleBrdf.dds";

            Texture::InitAsDDSTexture(*env_name, true, condition->env_SRV);
            Texture::InitAsDDSTexture(*specular_name, true,
                                      condition->irradiance_SRV);
            Texture::InitAsDDSTexture(*irradiance_name, true,
                                      condition->specular_SRV);
            Texture::InitAsDDSTexture(*brdf_name, true, condition->brdf_SRV);

            skyboxPSO = std::make_shared<dx12::SkyboxPSO>();
            skyboxPSO->Initialize();

            break;
        }
        case EnumStageType::eRender: {
            // skyboxPSO->Render(command_list, sampler_heap, const_buffer,
            //                        mesh->vertex_buffer_view,
            //                        mesh->index_buffer_view,
            //                        mesh->index_count);
            break;
        }
        default:
            break;
        }

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }

    std::shared_ptr<dx12::SkyboxPSO> skyboxPSO;
    std::shared_ptr<Model> skybox;
};
} // namespace core

#endif
