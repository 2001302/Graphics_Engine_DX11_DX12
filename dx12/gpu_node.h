#ifndef _GPU_NODE
#define _GPU_NODE

#include "mesh_util.h"
#include "shadow_map.h"
#include "skybox_renderer.h"
#include <behaviortree/behavior_tree_builder.h>

namespace graphics {
class ClearBufferNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        GpuCore::Instance().GetCommand()->Wait(D3D12_COMMAND_LIST_TYPE_DIRECT);

        auto context =
            GpuCore::Instance().GetCommand()->Begin<GraphicsCommandContext>(
                L"ClearBufferNode");

        context->TransitionResource(GpuBuffer::Instance().GetHDR(),
                                    D3D12_RESOURCE_STATE_RENDER_TARGET, true);

        context->ClearRenderTargetView(
            GpuBuffer::Instance().GetHDR()->GetRtvHandle());
        context->ClearDepthStencilView(
            GpuBuffer::Instance().GetDSV()->GetDsvHandle(),
            D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL);

        GpuCore::Instance().GetCommand()->Finish(context);

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class PresentNode : public common::BehaviorActionNode {

    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto context =
            GpuCore::Instance().GetCommand()->Begin<GraphicsCommandContext>(
                L"PresentNode");
        context->TransitionResource(GpuBuffer::Instance().GetDisplay(),
                                    D3D12_RESOURCE_STATE_PRESENT, true);
        GpuCore::Instance().GetCommand()->Finish(context);

        GpuCore::Instance().GetSwapChain()->Present(1, 0);
        GpuBuffer::Instance().GetDisplay()->MoveToNext();

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class GlobalConstantNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto targets = black_board->targets.get();
        auto condition = black_board->conditions.get();

        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {

            condition->global_consts.Initialize();

            auto context =
                GpuCore::Instance().GetCommand()->Begin<GraphicsCommandContext>(
                    L"SharedResource");

            SkyboxRenderer *skybox = nullptr;
            if (!targets->world->TryGet(skybox)) {

                auto mesh_data = GeometryGenerator::MakeBox(40.0f);
                std::reverse(mesh_data.indices.begin(),
                             mesh_data.indices.end());

                auto skybox_renderer = std::make_shared<SkyboxRenderer>();
                skybox_renderer->Initialize(
                    std::vector{mesh_data});
                targets->world->TryAdd(skybox_renderer);
            }

            ShadowMap *shadow = nullptr;
            if (!targets->world->TryGet(shadow)) {
                auto shadow_map = std::make_shared<ShadowMap>();
                shadow_map->Initialize();
                targets->world->TryAdd(shadow_map);
            }

            std::vector<D3D12_SAMPLER_DESC> sampler_desc{
                sampler::linearWrapSS,  sampler::linearClampSS,
                sampler::shadowPointSS, sampler::shadowCompareSS,
                sampler::pointWrapSS,   sampler::linearMirrorSS,
                sampler::pointClampSS};

            condition->shared_sampler = SamplerState::Create(sampler_desc);

            GpuCore::Instance().GetCommand()->Finish(context, true);
            break;
        }
        case EnumStageType::eUpdate: {

            const Vector3 eyeWorld = targets->camera->GetPosition();
            const Matrix viewRow = targets->camera->GetView();
            const Matrix projRow = targets->camera->GetProjection();

            condition->global_consts.GetCpu().eyeWorld = eyeWorld;
            condition->global_consts.GetCpu().view = viewRow.Transpose();
            condition->global_consts.GetCpu().proj = projRow.Transpose();
            condition->global_consts.GetCpu().invProj =
                projRow.Invert().Transpose();
            condition->global_consts.GetCpu().viewProj =
                (viewRow * projRow).Transpose();

            // used to shadow rendering
            condition->global_consts.GetCpu().invViewProj =
                condition->global_consts.GetCpu().viewProj.Invert();

            condition->global_consts.Upload();

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
