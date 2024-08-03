#ifndef _SHARED_RESOURCE_NODE
#define _SHARED_RESOURCE_NODE

#include "behavior_tree_builder.h"
#include "black_board.h"
#include "renderer.h"

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

            GraphicsUtil::CreateConstBuffer(GraphicsManager::Instance().device,
                                            manager->reflect_global_consts_CPU,
                                            manager->reflect_global_consts_GPU);

            // 그림자맵 렌더링할 때 사용할 GlobalConsts들 별도 생성
            for (int i = 0; i < MAX_LIGHTS; i++) {
                GraphicsUtil::CreateConstBuffer(
                    GraphicsManager::Instance().device,
                    manager->shadow_global_consts_CPU[i],
                    manager->shadow_global_consts_GPU[i]);
            }

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

            // 그림자 렌더링에 사용
            manager->global_consts_CPU.invViewProj =
                manager->global_consts_CPU.viewProj.Invert();

            manager->reflect_global_consts_CPU = manager->global_consts_CPU;
            memcpy(&manager->reflect_global_consts_CPU,
                   &manager->global_consts_CPU,
                   sizeof(manager->global_consts_CPU));
            manager->reflect_global_consts_CPU.view =
                (reflectRow * viewRow).Transpose();
            manager->reflect_global_consts_CPU.viewProj =
                (reflectRow * viewRow * projRow).Transpose();

            // 그림자 렌더링에 사용 (TODO: 광원의 위치도 반사시킨 후에 계산해야
            // 함)
            manager->reflect_global_consts_CPU.invViewProj =
                manager->reflect_global_consts_CPU.viewProj.Invert();

            GraphicsUtil::UpdateBuffer(
                GraphicsManager::Instance().device_context,
                manager->global_consts_CPU, manager->global_consts_GPU);

            GraphicsUtil::UpdateBuffer(
                GraphicsManager::Instance().device_context,
                manager->reflect_global_consts_CPU,
                manager->reflect_global_consts_GPU);
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