#ifndef _SKYBOXNODE
#define _SKYBOXNODE

#include "black_board.h"
#include "mesh_renderer.h"
#include <behavior_tree_builder.h>

namespace graphics {

class SkyboxNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->targets.get();

        switch (manager->stage_type) {
        case EnumStageType::eInitialize: {

            auto mesh_data = GeometryGenerator::MakeBox(40.0f);
            std::reverse(mesh_data.indices.begin(), mesh_data.indices.end());

            auto renderer =
                std::make_shared<MeshRenderer>(std::vector{mesh_data});

            manager->skybox = std::make_shared<Skybox>();
            manager->skybox->model = std::make_shared<Model>();
            manager->skybox->model->TryAdd(renderer);

            auto envFilename =
                L"./Assets/Textures/Cubemaps/HDRI/SampleEnvHDR.dds";
            auto specularFilename =
                L"./Assets/Textures/Cubemaps/HDRI/SampleSpecularHDR.dds";
            auto irradianceFilename =
                L"./Assets/Textures/Cubemaps/HDRI/SampleDiffuseHDR.dds";
            auto brdfFilename =
                L"./Assets/Textures/Cubemaps/HDRI/SampleBrdf.dds";

            graphics::Util::CreateDDSTexture(envFilename, true,
                                             manager->skybox->env_SRV);
            graphics::Util::CreateDDSTexture(specularFilename, true,
                                             manager->skybox->specular_SRV);
            graphics::Util::CreateDDSTexture(irradianceFilename, true,
                                             manager->skybox->irradiance_SRV);
            graphics::Util::CreateDDSTexture(brdfFilename, true,
                                             manager->skybox->brdf_SRV);
            break;
        }
        case EnumStageType::eRender: {

            graphics::Util::SetPipelineState(
                manager->draw_wire ? graphics::pipeline::skyboxWirePSO
                                   : graphics::pipeline::skyboxSolidPSO);
            MeshRenderer *renderer = nullptr;
            if (manager->skybox->model->TryGet(renderer)) {
                renderer->Render();
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