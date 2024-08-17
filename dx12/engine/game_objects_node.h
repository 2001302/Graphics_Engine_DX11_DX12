#ifndef _RENDERER_DRAW_NODE
#define _RENDERER_DRAW_NODE

#include "../foundation/behavior_tree_builder.h"
#include "black_board.h"
#include "mesh_renderer.h"
//#include "skinned_mesh_renderer.h"

namespace core {

class GameObjectNodeInvoker : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->job_context;
        auto gui = black_board->gui;

        switch (manager->stage_type) {
        case EnumStageType::eInitialize: {

            // additional object 1

            MeshData mesh = GeometryGenerator::MakeSphere(0.2f, 200, 200);
            Vector3 center(0.5f, 0.5f, 2.0f);

            auto renderer = std::make_shared<MeshRenderer>(std::vector{mesh});

            renderer->UpdateWorldRow(Matrix::CreateTranslation(center));
            renderer->material_consts.GetCpu().albedoFactor =
                Vector3(0.1f, 0.1f, 1.0f);
            renderer->material_consts.GetCpu().roughnessFactor = 0.2f;
            renderer->material_consts.GetCpu().metallicFactor = 0.6f;
            renderer->material_consts.GetCpu().emissionFactor = Vector3(0.0f);

            renderer->UpdateConstantBuffers();

            auto obj = std::make_shared<Model>();
            obj->AddComponent(EnumComponentType::eRenderer, renderer);

            manager->objects.insert({obj->GetEntityId(), obj});

            break;
        }
        case EnumStageType::eUpdate: {

            for (auto &i : manager->objects) {
                auto renderer = (MeshRenderer *)i.second->GetComponent(
                    EnumComponentType::eRenderer);
                renderer->UpdateConstantBuffers();
            }

            break;
        }
        case EnumStageType::eRender: {

            for (auto &i : manager->objects) {
                auto renderer = (MeshRenderer *)i.second->GetComponent(
                    EnumComponentType::eRenderer);

                std::vector<ComPtr<ID3D12Resource>> constantBuffers = {
                    manager->global_consts_GPU, renderer->mesh_consts.Get(),
                    renderer->material_consts.Get()};

                // 디스크립터 힙 생성 및 설정
                D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
                heapDesc.NumDescriptors = constantBuffers.size();
                heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

                dx12::ThrowIfFailed(
                    dx12::GpuCore::Instance().device->CreateDescriptorHeap(
                        &heapDesc,
                        IID_PPV_ARGS(&dx12::GpuCore::Instance().cbvHeap)));

                // 상수 버퍼 뷰(CBV) 생성
                D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc[3];
                CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle(
                    dx12::GpuCore::Instance()
                        .cbvHeap->GetCPUDescriptorHandleForHeapStart());

                for (int i = 0; i < constantBuffers.size(); ++i) {
                    cbvDesc[i].BufferLocation =
                        constantBuffers[i]->GetGPUVirtualAddress();
                    cbvDesc[i].SizeInBytes = 256;

                    dx12::GpuCore::Instance().device->CreateConstantBufferView(
                        &cbvDesc[i], cbvHandle);
                    cbvHandle.Offset(
                        dx12::GpuCore::Instance()
                            .device->GetDescriptorHandleIncrementSize(
                                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
                }
                // 상수 버퍼 바인딩
                ID3D12DescriptorHeap *descriptorHeaps[] = {
                    dx12::GpuCore::Instance().cbvHeap.Get()};
                dx12::GpuCore::Instance().commandList->SetDescriptorHeaps(
                    _countof(descriptorHeaps), descriptorHeaps);

                dx12::GpuCore::Instance().commandList->SetGraphicsRootSignature(
                    dx12::GpuCore::Instance().rootSignature.Get());

                for (int i = 0; i < constantBuffers.size(); ++i) {
                    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(
                        dx12::GpuCore::Instance()
                            .cbvHeap->GetGPUDescriptorHandleForHeapStart());
                    gpuHandle.Offset(
                        i, dx12::GpuCore::Instance()
                               .device->GetDescriptorHandleIncrementSize(
                                   D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
                    dx12::GpuCore::Instance()
                        .commandList->SetGraphicsRootDescriptorTable(i,
                                                                     gpuHandle);
                }

                renderer->Render();
            }

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