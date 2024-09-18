#ifndef _GPU_NODE
#define _GPU_NODE

#include "common/behavior_tree_builder.h"
#include "graphics_util.h"

namespace graphics {
class GpuInitializeNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        GpuCore::Instance().Initialize();

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class BeginInitNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        // GpuCore::Instance().GetCommandMgr()->Queue()

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class EndInitNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        // GpuCore::Instance().GetCommandMgr()->Queue()

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class BeginRenderNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        GpuCore::Instance().GetCommand()->Wait(D3D12_COMMAND_LIST_TYPE_DIRECT);

        //auto context =
        //    GpuCore::Instance().GetCommand()->Begin<GraphicsCommandContext>(
        //        L"BeginRenderNode");

        //context->SetDescriptorHeaps(
        //    std::vector{GpuCore::Instance().GetHeap().View(),
        //                GpuCore::Instance().GetHeap().Sampler()});

        //context->TransitionResource(GpuCore::Instance().GetBuffers().hdr_buffer,
        //                            D3D12_RESOURCE_STATE_RENDER_TARGET, true);

        //context->ClearRenderTargetView(
        //    GpuCore::Instance().GetBuffers().hdr_buffer);

        //context->ClearDepthStencilView(
        //    GpuCore::Instance().GetBuffers().dsv_buffer);

        //context->SetRenderTargetView(
        //    GpuCore::Instance().GetBuffers().hdr_buffer,
        //    GpuCore::Instance().GetBuffers().dsv_buffer);

        //GpuCore::Instance().GetCommand()->Finish(context, true);

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class PresentNode : public common::BehaviorActionNode {

    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto context =
            GpuCore::Instance().GetCommand()->Begin<GraphicsCommandContext>(
                L"PresentNode");
        context->TransitionResource(GpuCore::Instance().GetDisplay(),
                                    D3D12_RESOURCE_STATE_PRESENT, true);
        GpuCore::Instance().GetCommand()->Finish(context, true);

        GpuCore::Instance().GetSwapChain()->Present(1, 0);
        GpuCore::Instance().GetDisplay()->MoveToNext();

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class ResolveBuffer : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {
        // auto command_manager = GpuCore::Instance().GetCommand();

        // command_manager.GraphicsList()->TransitionResource(
        //     GpuCore::Instance().GetBuffers().hdr_buffer,
        //     D3D12_RESOURCE_STATE_RESOLVE_SOURCE, true);

        // command_manager.GraphicsList()->TransitionResource(
        //     GpuCore::Instance().GetBuffers().ldr_buffer,
        //     D3D12_RESOURCE_STATE_RESOLVE_DEST, true);

        // command_manager.GraphicsList()->ResolveSubresource(
        //     GpuCore::Instance().GetBuffers().ldr_buffer,
        //     GpuCore::Instance().GetBuffers().hdr_buffer,
        //     DXGI_FORMAT_R16G16B16A16_FLOAT);

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class SharedResourceNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto target = black_board->targets.get();
        auto condition = black_board->conditions.get();

        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {

            auto context =
                GpuCore::Instance().GetCommand()->Begin<CopyCommandContext>(
                    L"SharedResourceNode");

            // global constants
            condition->global_consts.Initialize();

            auto env = new TextureCube();
            auto specular = new TextureCube();
            auto diffuse = new TextureCube();
            auto brdf = new TextureCube();
            auto shadow = new Texture2D[MAX_LIGHTS];

            env->Create(GpuCore::Instance().GetDevice(),
                        GpuCore::Instance().GetHeap().View(),
                        L"./Assets/Textures/Cubemaps/HDRI/SampleEnvHDR.dds",
                        context->GetList(), true);
            specular->Create(
                GpuCore::Instance().GetDevice(),
                GpuCore::Instance().GetHeap().View(),
                L"./Assets/Textures/Cubemaps/HDRI/SampleSpecularHDR.dds",
                context->GetList(), true);
            diffuse->Create(
                GpuCore::Instance().GetDevice(),
                GpuCore::Instance().GetHeap().View(),
                L"./Assets/Textures/Cubemaps/HDRI/SampleDiffuseHDR.dds",
                context->GetList(), true);
            brdf->Create(GpuCore::Instance().GetDevice(),
                         GpuCore::Instance().GetHeap().View(),
                         L"./Assets/Textures/Cubemaps/HDRI/SampleBrdf.dds",
                         context->GetList(), true, true);

            std::vector<GpuResource *> tex = {env, specular, diffuse, brdf};

            for (int i = 0; i < MAX_LIGHTS; i++) {
                shadow[i].Create(GpuCore::Instance().GetDevice(),
                                 GpuCore::Instance().GetHeap().View(), 1024,
                                 1024, DXGI_FORMAT_R8G8B8A8_UNORM);
                tex.push_back(&shadow[i]);
            }

            condition->shared_texture = std::make_shared<GpuResourceList>(tex);

            std::vector<D3D12_SAMPLER_DESC> sampler_desc{
                sampler::linearWrapSS,  sampler::linearClampSS,
                sampler::shadowPointSS, sampler::shadowCompareSS,
                sampler::pointWrapSS,   sampler::linearMirrorSS,
                sampler::pointClampSS};

            condition->shared_sampler = std::make_shared<SamplerState>();
            condition->shared_sampler->Create(
                GpuCore::Instance().GetDevice(),
                GpuCore::Instance().GetHeap().Sampler(), sampler_desc);

            GpuCore::Instance().GetCommand()->Finish(context, true);

            condition->shared_texture->Allocate();
            condition->shared_sampler->Allocate();

            break;
        }
        case EnumStageType::eUpdate: {

            const Vector3 eyeWorld = target->camera->GetPosition();
            // const Matrix reflectRow =
            //     Matrix::CreateReflection(condition->ground->mirror_plane);
            const Matrix viewRow = target->camera->GetView();
            const Matrix projRow = target->camera->GetProjection();

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
