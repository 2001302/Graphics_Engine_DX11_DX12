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

        auto manager = black_board->job_context.get();

        switch (manager->stage_type) {
        case EnumStageType::eInitialize: {

            dx12::Util::CreateConstBuffer(manager->global_consts_CPU,
                                          manager->global_consts_GPU);

            // sampler
            D3D12_SAMPLER_DESC linearWrapSS;
            ZeroMemory(&linearWrapSS, sizeof(linearWrapSS));
            linearWrapSS.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            linearWrapSS.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            linearWrapSS.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            linearWrapSS.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            linearWrapSS.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
            linearWrapSS.MinLOD = 0;
            linearWrapSS.MaxLOD = D3D12_FLOAT32_MAX;

            D3D12_SAMPLER_DESC linearClampSS;
            ZeroMemory(&linearClampSS, sizeof(linearClampSS));
            linearClampSS.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            linearClampSS.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            linearClampSS.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            linearClampSS.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            linearClampSS.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
            linearClampSS.MinLOD = 0;
            linearClampSS.MaxLOD = D3D12_FLOAT32_MAX;

            D3D12_SAMPLER_DESC shadowPointSS;
            ZeroMemory(&shadowPointSS, sizeof(shadowPointSS));
            shadowPointSS.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
            shadowPointSS.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            shadowPointSS.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            shadowPointSS.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            shadowPointSS.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
            shadowPointSS.MinLOD = 0;
            shadowPointSS.MaxLOD = D3D12_FLOAT32_MAX;
            shadowPointSS.BorderColor[0] = 1.0f;

            D3D12_SAMPLER_DESC shadowCompareSS;
            ZeroMemory(&shadowCompareSS, sizeof(shadowCompareSS));
            shadowCompareSS.Filter =
                D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
            shadowCompareSS.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            shadowCompareSS.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            shadowCompareSS.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            shadowCompareSS.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
            shadowCompareSS.MinLOD = 0;
            shadowCompareSS.MaxLOD = D3D12_FLOAT32_MAX;
            shadowCompareSS.BorderColor[0] = 100.0f;

            //dummy 
            D3D12_SAMPLER_DESC pointWrapSS;
            ZeroMemory(&pointWrapSS, sizeof(pointWrapSS));
            pointWrapSS.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            pointWrapSS.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            pointWrapSS.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            pointWrapSS.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            pointWrapSS.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
            pointWrapSS.MinLOD = 0;
            pointWrapSS.MaxLOD = D3D12_FLOAT32_MAX;
            // dummy
            D3D12_SAMPLER_DESC linearMirrorSS;
            ZeroMemory(&linearMirrorSS, sizeof(linearMirrorSS));
            linearMirrorSS.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            linearMirrorSS.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            linearMirrorSS.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            linearMirrorSS.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            linearMirrorSS.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
            linearMirrorSS.MinLOD = 0;
            linearMirrorSS.MaxLOD = D3D12_FLOAT32_MAX;
            // dummy
            D3D12_SAMPLER_DESC pointClampSS;
            ZeroMemory(&pointClampSS, sizeof(pointClampSS));
            pointClampSS.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            pointClampSS.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            pointClampSS.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            pointClampSS.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            pointClampSS.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
            pointClampSS.MinLOD = 0;
            pointClampSS.MaxLOD = D3D12_FLOAT32_MAX;

            // s0~s3
            std::vector<D3D12_SAMPLER_DESC> sampleStates;
            sampleStates.push_back(linearWrapSS);
            sampleStates.push_back(linearClampSS);
            sampleStates.push_back(shadowPointSS);
            sampleStates.push_back(shadowCompareSS);
            sampleStates.push_back(pointWrapSS);
            sampleStates.push_back(linearMirrorSS);
            sampleStates.push_back(pointClampSS);

            D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
            samplerHeapDesc.NumDescriptors = (UINT)sampleStates.size();
            samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
            samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

            dx12::ThrowIfFailed(
                dx12::GpuCore::Instance().device->CreateDescriptorHeap(
                    &samplerHeapDesc, IID_PPV_ARGS(&manager->samplerHeap)));

            CD3DX12_CPU_DESCRIPTOR_HANDLE handle(
                manager->samplerHeap->GetCPUDescriptorHandleForHeapStart());
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

            const Vector3 eyeWorld = manager->camera->GetPosition();
            // const Matrix reflectRow =
            //     Matrix::CreateReflection(manager->ground->mirror_plane);
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

            dx12::Util::UpdateBuffer(manager->global_consts_CPU,
                                     manager->global_consts_GPU);

            break;
        }
        case EnumStageType::eRender: {

            // dx11::GpuCore::Instance().SetMainViewport();

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
            //     manager->skybox->env_SRV.Get(),
            //     manager->skybox->specular_SRV.Get(),
            //     manager->skybox->irradiance_SRV.Get(),
            //     manager->skybox->brdf_SRV.Get()};
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