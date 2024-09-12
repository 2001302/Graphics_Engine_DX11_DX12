#ifndef _GUINODE
#define _GUINODE

#include "black_board.h"
#include "foundation/behavior_tree_builder.h"

namespace graphics {

class GuiNodeInvoker : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        //auto black_board = dynamic_cast<BlackBoard *>(data_block);
        //assert(black_board != nullptr);

        //auto target = black_board->targets.get();
        //auto condition = black_board->conditions.get();

        //auto gui = black_board->gui;
        //auto command_pool = condition->command_pool;

        //switch (condition->stage_type) {
        //case EnumStageType::eInitialize: {
        //    gui->Start();
        //    gui->PushInfoItem(target);

        //    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        //    desc.NumDescriptors = 1;
        //    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        //    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        //    ThrowIfFailed(GpuDevice::Get().device->CreateDescriptorHeap(
        //        &desc, IID_PPV_ARGS(&cbvHeap)));
        //    UINT descSize =
        //        GpuDevice::Get().device->GetDescriptorHandleIncrementSize(
        //            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        //    ImGui_ImplWin32_Init(foundation::Env::Instance().main_window);
        //    ImGui_ImplDX12_Init(GpuDevice::Get().device.Get(), 3,
        //                        DXGI_FORMAT_R8G8B8A8_UNORM, cbvHeap.Get(),
        //                        cbvHeap->GetCPUDescriptorHandleForHeapStart(),
        //                        cbvHeap->GetGPUDescriptorHandleForHeapStart());

        //    break;
        //}
        //case EnumStageType::eRender: {
        //    gui->Frame();
        //    gui->ClearNodeItem();

        //    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        //        GpuDevice::Get()
        //            .resource_FLIP[GpuDevice::Get().frame_index]
        //            .Get(),
        //        D3D12_RESOURCE_STATE_COMMON,
        //        D3D12_RESOURCE_STATE_RENDER_TARGET);
        //    command_pool->Get(0)->ResourceBarrier(1, &barrier);

        //    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle =
        //        GpuDevice::Get().GetHandleFLIP();
        //    command_pool->Get(0)->RSSetViewports(1,
        //                                         &GpuDevice::Get().viewport);
        //    command_pool->Get(0)->OMSetRenderTargets(1, &rtvHandle, false,
        //                                             nullptr);
        //    ID3D12DescriptorHeap *descriptorHeaps[] = {cbvHeap.Get()};
        //    command_pool->Get(0)->SetDescriptorHeaps(_countof(descriptorHeaps),
        //                                             descriptorHeaps);

        //    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(),
        //                                  command_pool->Get(0).Get());

        //    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        //        GpuDevice::Get()
        //            .resource_FLIP[GpuDevice::Get().frame_index]
        //            .Get(),
        //        D3D12_RESOURCE_STATE_RENDER_TARGET,
        //        D3D12_RESOURCE_STATE_COMMON);
        //    command_pool->Get(0)->ResourceBarrier(1, &barrier);
        //    break;
        //}
        //default:
        //    break;
        //}

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
    ComPtr<ID3D12DescriptorHeap> cbvHeap;
};

} // namespace core

#endif