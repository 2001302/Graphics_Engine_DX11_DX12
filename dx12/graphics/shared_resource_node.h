#ifndef _SHARED_RESOURCE_NODE
#define _SHARED_RESOURCE_NODE

#include "black_board.h"
#include "foundation/behavior_tree_builder.h"
#include "mesh_renderer.h"
#include "sampler_state.h"

namespace graphics {

class SharedResourceNodeInvoker : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto target = black_board->targets.get();
        auto condition = black_board->conditions.get();
        auto command_list = condition->command_pool->Get(0);

        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {

            // global constants
            condition->global_consts.Initialize();

            // gpu heap
            condition->gpu_heap = std::make_shared<GpuHeap>(
                1024, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0);
            condition->sampler_heap = std::make_shared<GpuHeap>(
                7, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 0);

            condition->shared_texture =
                std::make_shared<GpuResourceList>(condition->gpu_heap.get());
            condition->shared_texture->Add(std::make_shared<TextureCube>(
                L"./Assets/Textures/Cubemaps/HDRI/SampleEnvHDR.dds",
                command_list, true));
            condition->shared_texture->Add(std::make_shared<TextureCube>(
                L"./Assets/Textures/Cubemaps/HDRI/SampleSpecularHDR.dds",
                command_list, true));
            condition->shared_texture->Add(std::make_shared<TextureCube>(
                L"./Assets/Textures/Cubemaps/HDRI/SampleDiffuseHDR.dds",
                command_list, true));
            condition->shared_texture->Add(std::make_shared<TextureCube>(
                L"./Assets/Textures/Cubemaps/HDRI/SampleBrdf.dds", command_list,
                true, true));

            for (int i = 0; i < MAX_LIGHTS; i++) {
                condition->shared_texture->Add(std::make_shared<Texture2D>(
                    1024, 1024, DXGI_FORMAT_R8G8B8A8_UNORM));
            }
            condition->shared_texture->Allocate();

            auto samplers = std::make_shared<GpuResourceList>(
                condition->sampler_heap.get());
            samplers->Add(
                std::make_shared<SamplerState>(sampler::linearWrapSS));
            samplers->Add(
                std::make_shared<SamplerState>(sampler::linearClampSS));
            samplers->Add(
                std::make_shared<SamplerState>(sampler::shadowPointSS));
            samplers->Add(
                std::make_shared<SamplerState>(sampler::shadowCompareSS));
            samplers->Add(std::make_shared<SamplerState>(sampler::pointWrapSS));
            samplers->Add(
                std::make_shared<SamplerState>(sampler::linearMirrorSS));
            samplers->Add(
                std::make_shared<SamplerState>(sampler::pointClampSS));
            samplers->Allocate();

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

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace graphics

#endif
