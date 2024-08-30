#ifndef _SHARED_RESOURCE_NODE
#define _SHARED_RESOURCE_NODE

#include "../foundation/behavior_tree_builder.h"
#include "black_board.h"
#include "mesh_renderer.h"

namespace core {

class SharedResourceNodeInvoker : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto target = black_board->targets.get();
        auto condition = black_board->conditions.get();

        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {

            condition->global_consts.Initialize();

            // s0~s6
            std::vector<D3D12_SAMPLER_DESC> sampleStates;
            sampleStates.push_back(dx12::sampler::linearWrapSS);
            sampleStates.push_back(dx12::sampler::linearClampSS);
            sampleStates.push_back(dx12::sampler::shadowPointSS);
            sampleStates.push_back(dx12::sampler::shadowCompareSS);
            sampleStates.push_back(dx12::sampler::pointWrapSS);
            sampleStates.push_back(dx12::sampler::linearMirrorSS);
            sampleStates.push_back(dx12::sampler::pointClampSS);

            D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
            samplerHeapDesc.NumDescriptors = (UINT)sampleStates.size();
            samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
            samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

            dx12::ThrowIfFailed(
                dx12::GpuCore::Instance().device->CreateDescriptorHeap(
                    &samplerHeapDesc, IID_PPV_ARGS(&condition->sampler_heap)));

            CD3DX12_CPU_DESCRIPTOR_HANDLE handle(
                condition->sampler_heap->GetCPUDescriptorHandleForHeapStart());
            UINT incrementSize = dx12::GpuCore::Instance()
                                     .device->GetDescriptorHandleIncrementSize(
                                         D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

            for (int i = 0; i < sampleStates.size(); ++i) {
                dx12::GpuCore::Instance().device->CreateSampler(
                    &sampleStates[i], handle);
                handle.Offset(incrementSize);
            }

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
        case EnumStageType::eRender: {
            // dx11::GpuCore::Instance().device_context->VSSetSamplers(
            //     0, UINT(dx11::pso::sampleStates.size()),
            //     dx11::pso::sampleStates.data());
            // dx11::GpuCore::Instance().device_context->PSSetSamplers(
            //     0, UINT(dx11::pso::sampleStates.size()),
            //     dx11::pso::sampleStates.data());
            // dx11::GpuCore::Instance().device_context->CSSetSamplers(
            //     0, UINT(dx11::pso::sampleStates.size()),
            //     dx11::pso::sampleStates.data());

            //// Shared textures: start from register(t10) in 'Common.hlsli'
            // std::vector<ID3D11ShaderResourceView *> commonSRVs = {
            //     condition->skybox->env_SRV.Get(),
            //     condition->skybox->specular_SRV.Get(),
            //     condition->skybox->irradiance_SRV.Get(),
            //     condition->skybox->brdf_SRV.Get()};
            // dx11::GpuCore::Instance()
            //     .device_context->PSSetShaderResources(
            //         10, UINT(commonSRVs.size()), commonSRVs.data());

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
