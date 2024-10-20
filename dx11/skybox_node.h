#ifndef _SKYBOXNODE
#define _SKYBOXNODE

#include "black_board.h"
#include "mesh_renderer.h"
#include "skybox_renderer.h"
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
                std::make_shared<SkyboxRenderer>(std::vector{mesh_data});

            auto envFilename =
                L"../Assets/Textures/Cubemaps/HDRI/SampleEnvHDR.dds";
            auto specularFilename =
                L"../Assets/Textures/Cubemaps/HDRI/SampleSpecularHDR.dds";
            auto irradianceFilename =
                L"../Assets/Textures/Cubemaps/HDRI/SampleDiffuseHDR.dds";
            auto brdfFilename =
                L"../Assets/Textures/Cubemaps/HDRI/SampleBrdf.dds";

            graphics::Util::CreateDDSTexture(envFilename, true,
                                             renderer->env_SRV);
            graphics::Util::CreateDDSTexture(specularFilename, true,
                                             renderer->specular_SRV);
            graphics::Util::CreateDDSTexture(irradianceFilename, true,
                                             renderer->irradiance_SRV);
            graphics::Util::CreateDDSTexture(brdfFilename, true,
                                             renderer->brdf_SRV);

            manager->skybox = std::make_shared<Model>();
            manager->skybox->TryAdd(renderer);
            break;
        }
        case EnumStageType::eRender: {

            graphics::Util::SetPipelineState(
                manager->draw_wire ? graphics::pipeline::skyboxWirePSO
                                   : graphics::pipeline::skyboxSolidPSO);
            SkyboxRenderer *renderer = nullptr;
            if (manager->skybox->TryGet(renderer)) {
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