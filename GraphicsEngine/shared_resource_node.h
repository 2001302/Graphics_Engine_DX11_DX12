#ifndef _SHARED_RESOURCE_NODE
#define _SHARED_RESOURCE_NODE

#include "behavior_tree_builder.h"
#include "black_board.h"
#include "mesh_renderer.h"

namespace core {

class SharedResourceNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->job_context.get();

        switch (manager->stage_type) {
        case EnumStageType::eInitialize: {

            GraphicsUtil::CreateConstBuffer(manager->global_consts_CPU,
                                            manager->global_consts_GPU);

            break;
        }
        case EnumStageType::eUpdate: {

            const Vector3 eyeWorld = manager->camera->GetPosition();
            const Matrix reflectRow =
                Matrix::CreateReflection(manager->ground->mirror_plane);
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

            GraphicsUtil::UpdateBuffer(manager->global_consts_CPU,
                                       manager->global_consts_GPU);

            break;
        }
        case EnumStageType::eRender: {
            GraphicsCore::Instance().SetMainViewport();

            GraphicsCore::Instance().device_context->VSSetSamplers(
                0, UINT(graphics::sampleStates.size()),
                graphics::sampleStates.data());
            GraphicsCore::Instance().device_context->PSSetSamplers(
                0, UINT(graphics::sampleStates.size()),
                graphics::sampleStates.data());
            GraphicsCore::Instance().device_context->CSSetSamplers(
                0, UINT(graphics::sampleStates.size()),
                graphics::sampleStates.data());

            // Shared textures: start from register(t10) in 'Common.hlsli'
            std::vector<ID3D11ShaderResourceView *> commonSRVs = {
                manager->skybox->env_SRV.Get(),
                manager->skybox->specular_SRV.Get(),
                manager->skybox->irradiance_SRV.Get(),
                manager->skybox->brdf_SRV.Get()};
            GraphicsCore::Instance().device_context->PSSetShaderResources(
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