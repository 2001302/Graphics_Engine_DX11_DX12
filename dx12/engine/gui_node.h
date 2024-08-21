#ifndef _GUINODE
#define _GUINODE

#include "../foundation/behavior_tree_builder.h"
#include "black_board.h"

namespace core {

class GuiNodeInvoker : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->job_context.get();
        auto gui = black_board->gui;

        switch (manager->stage_type) {
        case EnumStageType::eInitialize: {
            gui->Start();
            gui->PushInfoItem(manager);

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
