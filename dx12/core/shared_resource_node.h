#ifndef _SHARED_RESOURCE_NODE
#define _SHARED_RESOURCE_NODE

#include "foundation/behavior_tree_builder.h"
#include "black_board.h"
#include "mesh_renderer.h"

namespace core {

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
            condition->gpu_heap = std::make_shared<dx12::GpuHeap>(
                1024, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0);

            condition->shared_texture = std::make_shared<dx12::GpuBufferList>(
                condition->gpu_heap.get());
            condition->shared_texture->Add(std::make_shared<dx12::TextureCube>(
                L"./Assets/Textures/Cubemaps/HDRI/SampleEnvHDR.dds",
                command_list, true));
            condition->shared_texture->Add(std::make_shared<dx12::TextureCube>(
                L"./Assets/Textures/Cubemaps/HDRI/SampleSpecularHDR.dds",
                command_list, true));
            condition->shared_texture->Add(std::make_shared<dx12::TextureCube>(
                L"./Assets/Textures/Cubemaps/HDRI/SampleDiffuseHDR.dds",
                command_list, true));
            condition->shared_texture->Add(std::make_shared<dx12::TextureCube>(
                L"./Assets/Textures/Cubemaps/HDRI/SampleBrdf.dds", command_list,
                true, true));

            for (int i = 0; i < MAX_LIGHTS; i++) {
                condition->shared_texture->Add(
                    std::make_shared<dx12::Texture2D>(
                        1024, 1024, DXGI_FORMAT_R8G8B8A8_UNORM));
            }
            condition->shared_texture->Allocate();

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

} // namespace core

#endif
