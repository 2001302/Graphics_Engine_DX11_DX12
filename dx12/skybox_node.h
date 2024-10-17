#ifndef _SKYBOX_NODE
#define _SKYBOX_NODE

#include "black_board.h"
#include "skybox_pso.h"
#include "skybox_renderer.h"
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

            auto component = std::make_shared<SkyboxRenderer>();
            component->Initialize(std::vector{mesh_data});
            component->SetSkyboxTexture(
                L"./Assets/Textures/Cubemaps/HDRI/SampleEnvHDR.dds",
                L"./Assets/Textures/Cubemaps/HDRI/SampleSpecularHDR.dds",
                L"./Assets/Textures/Cubemaps/HDRI/SampleDiffuseHDR.dds",
                L"./Assets/Textures/Cubemaps/HDRI/SampleBrdf.dds");

            targets->skybox = std::make_shared<common::Model>();
            targets->skybox->TryAdd(component);

            std::vector<GpuResource *> shadow = {};
            for (int i = 0; i < MAX_LIGHTS; i++) {
                shadow.push_back(std::move(DepthBuffer::Create(
                    1024, 1024, DXGI_FORMAT_D24_UNORM_S8_UINT)));
            }
            condition->shadow_texture = new GpuResourceList(shadow);

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
            if (targets->skybox->TryGet(component)) {
                skyboxPSO->Render(condition->shared_sampler,
                                  targets->skybox.get(),
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
