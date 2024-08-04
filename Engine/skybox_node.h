#ifndef _SKYBOXNODE
#define _SKYBOXNODE

#include "behavior_tree_builder.h"
#include "black_board.h"
#include "mesh_renderer.h"

namespace engine {

class SkyboxNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block.get();

        switch (manager->stage_type) {
        case EnumStageType::eInitialize: {

            auto mesh_data = GeometryGenerator::MakeBox(40.0f);
            std::reverse(mesh_data.indices.begin(), mesh_data.indices.end());

            auto renderer = std::make_shared<MeshRenderer>(
                GraphicsManager::Instance().device,
                GraphicsManager::Instance().device_context,
                std::vector{mesh_data});

            manager->skybox = std::make_shared<Model>();
            manager->skybox->AddComponent(EnumComponentType::eRenderer,
                                          renderer);

            auto envFilename =
                L"./Assets/Textures/Cubemaps/HDRI/SampleEnvHDR.dds";
            auto specularFilename =
                L"./Assets/Textures/Cubemaps/HDRI/SampleSpecularHDR.dds";
            auto irradianceFilename =
                L"./Assets/Textures/Cubemaps/HDRI/SampleDiffuseHDR.dds";
            auto brdfFilename =
                L"./Assets/Textures/Cubemaps/HDRI/SampleBrdf.dds";

            GraphicsUtil::CreateDDSTexture(GraphicsManager::Instance().device,
                                           envFilename, true, manager->env_SRV);
            GraphicsUtil::CreateDDSTexture(GraphicsManager::Instance().device,
                                           specularFilename, true,
                                           manager->specular_SRV);
            GraphicsUtil::CreateDDSTexture(GraphicsManager::Instance().device,
                                           irradianceFilename, true,
                                           manager->irradiance_SRV);
            GraphicsUtil::CreateDDSTexture(GraphicsManager::Instance().device,
                                           brdfFilename, true,
                                           manager->brdf_SRV);
            break;
        }
        case EnumStageType::eRender: {

            GraphicsManager::Instance().SetPipelineState(
                manager->draw_wire ? Graphics::skyboxWirePSO
                                   : Graphics::skyboxSolidPSO);
            auto renderer = (MeshRenderer *)manager->skybox->GetComponent(
                EnumComponentType::eRenderer);
            renderer->Render(GraphicsManager::Instance().device_context);
            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace engine

#endif