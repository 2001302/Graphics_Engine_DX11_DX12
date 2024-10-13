#ifndef _SHARED_RESOURCE_NODE
#define _SHARED_RESOURCE_NODE

#include <behavior_tree_builder.h>
#include "black_board.h"
#include "mesh_renderer.h"

namespace graphics {

class SharedResourceNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->targets.get();

        switch (manager->stage_type) {
        case EnumStageType::eInitialize: {

            graphics::Util::CreateConstBuffer(manager->global_consts_CPU,
                                              manager->global_consts_GPU);

            break;
        }
        case EnumStageType::eUpdate: {

            const Vector3 eyeWorld = manager->camera->GetPosition();
            const Matrix viewRow = manager->camera->GetView();
            const Matrix projRow = manager->camera->GetProjection();

            manager->global_consts_CPU.eyeWorld = eyeWorld;
            manager->global_consts_CPU.view = viewRow.Transpose();
            manager->global_consts_CPU.proj = projRow.Transpose();
            manager->global_consts_CPU.invProj = projRow.Invert().Transpose();
            manager->global_consts_CPU.viewProj =
                (viewRow * projRow).Transpose();

            // used to shadow rendering
            manager->global_consts_CPU.invViewProj =
                manager->global_consts_CPU.viewProj.Invert();

            graphics::Util::UpdateBuffer(manager->global_consts_CPU,
                                         manager->global_consts_GPU);

            break;
        }
        case EnumStageType::eRender: {
            graphics::GpuCore::Instance().SetMainViewport();

            graphics::GpuCore::Instance().device_context->VSSetSamplers(
                0, UINT(graphics::pipeline::sampleStates.size()),
                graphics::pipeline::sampleStates.data());
            graphics::GpuCore::Instance().device_context->PSSetSamplers(
                0, UINT(graphics::pipeline::sampleStates.size()),
                graphics::pipeline::sampleStates.data());
            graphics::GpuCore::Instance().device_context->CSSetSamplers(
                0, UINT(graphics::pipeline::sampleStates.size()),
                graphics::pipeline::sampleStates.data());

            // Shared textures: start from register(t10) in 'Common.hlsli'
            std::vector<ID3D11ShaderResourceView *> commonSRVs = {
                manager->skybox->env_SRV.Get(),
                manager->skybox->specular_SRV.Get(),
                manager->skybox->irradiance_SRV.Get(),
                manager->skybox->brdf_SRV.Get()};
            graphics::GpuCore::Instance()
                .device_context->PSSetShaderResources(
                    10, UINT(commonSRVs.size()), commonSRVs.data());

            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace core

#endif