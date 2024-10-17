#ifndef _SKYBOX_NODE
#define _SKYBOX_NODE

#include "black_board.h"
#include "skybox_pso.h"
#include "skybox_renderer.h"
#include "shadow_map.h"
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
            auto context =
                GpuCore::Instance().GetCommand()->Begin<GraphicsCommandContext>(
                    L"SharedResourceNode");

            skyboxPSO = std::make_shared<SolidSkyboxPSO>();
            skyboxPSO->Initialize();

            auto mesh_data = GeometryGenerator::MakeBox(40.0f);
            std::reverse(mesh_data.indices.begin(), mesh_data.indices.end());

            auto skybox_renderer = std::make_shared<SkyboxRenderer>();
            skybox_renderer->Initialize(std::vector{mesh_data});
            skybox_renderer->SetSkyboxTexture(
                L"./Assets/Textures/Cubemaps/HDRI/SampleEnvHDR.dds",
                L"./Assets/Textures/Cubemaps/HDRI/SampleSpecularHDR.dds",
                L"./Assets/Textures/Cubemaps/HDRI/SampleDiffuseHDR.dds",
                L"./Assets/Textures/Cubemaps/HDRI/SampleBrdf.dds");

            auto shadow_map = std::make_shared<ShadowMap>();
            shadow_map->Initialize();

            targets->world = std::make_shared<common::Model>();
            targets->world->TryAdd(skybox_renderer);
            targets->world->TryAdd(shadow_map);

            std::vector<D3D12_SAMPLER_DESC> sampler_desc{
                sampler::linearWrapSS,  sampler::linearClampSS,
                sampler::shadowPointSS, sampler::shadowCompareSS,
                sampler::pointWrapSS,   sampler::linearMirrorSS,
                sampler::pointClampSS};

            condition->shared_sampler = SamplerState::Create(sampler_desc);

            GpuCore::Instance().GetCommand()->Finish(context, true);

            break;
        }
        case EnumStageType::eRender: {
            SkyboxRenderer *component = nullptr;
            if (targets->world->TryGet(component)) {
                skyboxPSO->Render(condition->shared_sampler,
                                  targets->world.get(),
                                  condition->global_consts.Get(), component);
            }
            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }

    std::shared_ptr<SolidSkyboxPSO> skyboxPSO;
};
} // namespace graphics

#endif
