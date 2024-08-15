#ifndef _SKYBOXNODE
#define _SKYBOXNODE

#include "behavior_tree_builder.h"
#include "black_board.h"
#include "mesh_renderer.h"

namespace core {

class SkyboxNodeInvoker : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->job_context.get();

        switch (manager->stage_type) {
        case EnumStageType::eInitialize: {

            auto mesh_data = GeometryGenerator::MakeBox(40.0f);
            std::reverse(mesh_data.indices.begin(), mesh_data.indices.end());

            auto renderer =
                std::make_shared<MeshRenderer>(std::vector{mesh_data});

            manager->skybox = std::make_shared<Skybox>();
            manager->skybox->model = std::make_shared<Model>();
            manager->skybox->model->AddComponent(EnumComponentType::eRenderer,
                                                 renderer);

            auto envFilename =
                L"./Assets/Textures/Cubemaps/HDRI/SampleEnvHDR.dds";
            auto specularFilename =
                L"./Assets/Textures/Cubemaps/HDRI/SampleSpecularHDR.dds";
            auto irradianceFilename =
                L"./Assets/Textures/Cubemaps/HDRI/SampleDiffuseHDR.dds";
            auto brdfFilename =
                L"./Assets/Textures/Cubemaps/HDRI/SampleBrdf.dds";

            dx11::Util::CreateDDSTexture(envFilename, true,
                                             manager->skybox->env_SRV);
            dx11::Util::CreateDDSTexture(specularFilename, true,
                                             manager->skybox->specular_SRV);
            dx11::Util::CreateDDSTexture(irradianceFilename, true,
                                             manager->skybox->irradiance_SRV);
            dx11::Util::CreateDDSTexture(brdfFilename, true,
                                             manager->skybox->brdf_SRV);
            break;
        }
        case EnumStageType::eRender: {

            dx11::Util::SetPipelineState(
                manager->draw_wire ? dx11::pso::skyboxWirePSO
                                   : dx11::pso::skyboxSolidPSO);
            auto renderer =
                (MeshRenderer *)manager->skybox->model->GetComponent(
                    EnumComponentType::eRenderer);
            renderer->Render();
            break;
        }
        default:
            break;
        }

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace core

#endif