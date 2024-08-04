#ifndef _SHARED_RESOURCE_NODE
#define _SHARED_RESOURCE_NODE

#include "behavior_tree_builder.h"
#include "black_board.h"
#include "mesh_renderer.h"

namespace engine {

class SharedResourceNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block.get();

        switch (manager->stage_type) {
        case EnumStageType::eInitialize: {

            GraphicsUtil::CreateConstBuffer(GraphicsManager::Instance().device,
                                            manager->global_consts_CPU,
                                            manager->global_consts_GPU);

            break;
        }
        case EnumStageType::eUpdate: {

            const Vector3 eyeWorld = manager->camera->GetPosition();
            const Matrix reflectRow =
                Matrix::CreateReflection(manager->mirror_plane);
            const Matrix viewRow = manager->camera->GetView();
            const Matrix projRow = manager->camera->GetProjection();

            manager->global_consts_CPU.eyeWorld = eyeWorld;
            manager->global_consts_CPU.view = viewRow.Transpose();
            manager->global_consts_CPU.proj = projRow.Transpose();
            manager->global_consts_CPU.invProj = projRow.Invert().Transpose();
            manager->global_consts_CPU.viewProj =
                (viewRow * projRow).Transpose();

            //used to shadow rendering
            manager->global_consts_CPU.invViewProj =
                manager->global_consts_CPU.viewProj.Invert();

            GraphicsUtil::UpdateBuffer(
                GraphicsManager::Instance().device_context,
                manager->global_consts_CPU, manager->global_consts_GPU);

            break;
        }
        case EnumStageType::eRender: 
        {
            GraphicsManager::Instance().SetMainViewport();

            GraphicsManager::Instance().device_context->VSSetSamplers(
                0, UINT(Graphics::sampleStates.size()),
                Graphics::sampleStates.data());
            GraphicsManager::Instance().device_context->PSSetSamplers(
                0, UINT(Graphics::sampleStates.size()),
                Graphics::sampleStates.data());
            GraphicsManager::Instance().device_context->CSSetSamplers(
                0, UINT(Graphics::sampleStates.size()),
                Graphics::sampleStates.data());

            // Shared textures: start from register(t10) in 'Common.hlsli'
            std::vector<ID3D11ShaderResourceView *> commonSRVs = {
                manager->env_SRV.Get(), manager->specular_SRV.Get(),
                manager->irradiance_SRV.Get(), manager->brdf_SRV.Get()};
            GraphicsManager::Instance().device_context->PSSetShaderResources(
                10, UINT(commonSRVs.size()), commonSRVs.data());

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