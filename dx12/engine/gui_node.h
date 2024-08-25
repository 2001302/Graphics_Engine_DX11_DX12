#ifndef _GUINODE
#define _GUINODE

#include "../foundation/behavior_tree_builder.h"
#include "black_board.h"

namespace core {

class GuiNodeInvoker : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto target = black_board->targets.get();
        auto condition = black_board->conditions.get();

        auto gui = condition->gui;
        auto command_pool = condition->command_pool;

        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {
            gui->Start();
            gui->PushInfoItem(target);

            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.NumDescriptors = 1;
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            dx12::ThrowIfFailed(
                dx12::GpuCore::Instance().device->CreateDescriptorHeap(
                    &desc, IID_PPV_ARGS(&cbvHeap)));
            UINT descSize = dx12::GpuCore::Instance()
                                .device->GetDescriptorHandleIncrementSize(
                                    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            ImGui_ImplWin32_Init(foundation::Env::Instance().main_window);
            ImGui_ImplDX12_Init(dx12::GpuCore::Instance().device.Get(), 3,
                                DXGI_FORMAT_R8G8B8A8_UNORM, cbvHeap.Get(),
                                cbvHeap->GetCPUDescriptorHandleForHeapStart(),
                                cbvHeap->GetGPUDescriptorHandleForHeapStart());

            break;
        }
        case EnumStageType::eRender: {
            gui->Frame();
            gui->ClearNodeItem();

            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle =
                dx12::GpuCore::Instance().GetHandleRTV();
            command_pool->Get(1)->SetName(L"ImGui");
            command_pool->Get(1)->RSSetViewports(
                1, &dx12::GpuCore::Instance().viewport);
            command_pool->Get(1)->OMSetRenderTargets(1, &rtvHandle, false,
                                                     nullptr);
            ID3D12DescriptorHeap *descriptorHeaps[] = {cbvHeap.Get()};
            command_pool->Get(1)->SetDescriptorHeaps(_countof(descriptorHeaps),
                                            descriptorHeaps);

            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(),
                                          command_pool->Get(1).Get());
            break;
        }
        default:
            break;
        }

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
    ComPtr<ID3D12DescriptorHeap> cbvHeap;
};

} // namespace core

#endif
