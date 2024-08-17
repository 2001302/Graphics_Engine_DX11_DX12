#include "engine.h"
// #include "behavior_tree_builder.h"

namespace core {

Engine::Engine() {
    // black_board = std::make_shared<BlackBoard>();
    // message_receiver = std::make_unique<MessageReceiver>();
    gui = std::make_shared<foundation::SettingUi>();
};

bool Engine::Start() {

    Platform::Start();

    dx12::GpuCore::Instance().Initialize();
    gui->Start();
    // black_board->job_context->stage_type = EnumStageType::eInitialize;

    //// clang-format off
    // auto tree = std::make_shared<foundation::BehaviorTreeBuilder>();
    // tree->Build(black_board.get())
    //     ->Sequence()
    //         ->Excute(shadow_effect_node)
    //         ->Excute(light_node)
    //         ->Excute(camera_node)
    //         ->Excute(skybox_node)
    //         ->Excute(mirror_effect_node)
    //         ->Excute(shared_resource_node)
    //         ->Excute(post_processing)
    //         ->Excute(game_objects_node)
    //         ->Excute(player_node)
    //         ->Excute(imgui_node)
    //     ->Close()
    //->Run();
    //// clang-format on

    OnFrame();

    return true;
}

bool Engine::OnFrame() {

    OnUpdate(ImGui::GetIO().DeltaTime);
    OnRender();

    return true;
}

bool Engine::OnUpdate(float dt) {

    // black_board->job_context->dt = dt;
    // black_board->job_context->stage_type = EnumStageType::eUpdate;

    //// clang-format off
    // auto tree = std::make_shared<foundation::BehaviorTreeBuilder>();
    // tree->Build(black_board.get())
    //     ->Sequence()
    //         ->Excute(camera_node)
    //         ->Excute(light_node)
    //         ->Excute(shared_resource_node)
    //         ->Excute(shadow_effect_node)
    //         ->Excute(mirror_effect_node)
    //         ->Excute(game_objects_node)
    //         ->Excute(player_node)
    //     ->Close()
    //->Run();
    //// clang-format on

    return true;
}
void WaitForPreviousFrame() {
    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. The
    // D3D12HelloFrameBuffering sample illustrates how to use fences for
    // efficient resource usage and to maximize GPU utilization.

    // Signal and increment the fence value.
    const UINT64 fence = dx12::GpuCore::Instance().fenceValue;
    dx12::GpuCore::Instance().command_queue->Signal(
        dx12::GpuCore::Instance().fence.Get(), fence);
    dx12::GpuCore::Instance().fenceValue++;

    // Wait until the previous frame is finished.
    if (dx12::GpuCore::Instance().fence->GetCompletedValue() < fence) {
        dx12::GpuCore::Instance().fence->SetEventOnCompletion(
            fence, dx12::GpuCore::Instance().fenceEvent);
        WaitForSingleObject(dx12::GpuCore::Instance().fenceEvent, INFINITE);
    }

    dx12::GpuCore::Instance().frameIndex =
        dx12::GpuCore::Instance().swap_chain->GetCurrentBackBufferIndex();
}

bool Engine::OnRender() {

    // black_board->job_context->stage_type = EnumStageType::eRender;

    //// clang-format off
    // auto tree = std::make_shared<foundation::BehaviorTreeBuilder>();
    // tree->Build(black_board.get())
    //     ->Sequence()
    //         ->Excute(shared_resource_node)
    //         ->Excute(shadow_effect_node)
    //         ->Excute(light_node)
    //         ->Excute(game_objects_node)
    //         ->Excute(player_node)
    //         ->Excute(camera_node)
    //         ->Excute(skybox_node)
    //         ->Excute(mirror_effect_node)
    //         ->Excute(resolve_buffer)
    //         ->Excute(post_processing)
    //         ->Excute(imgui_node)
    //         ->Excute(present)
    //     ->Close()
    //->Run();
    //// clang-format on

    {
        dx12::GpuCore::Instance().commandAllocator->Reset();
        dx12::GpuCore::Instance().commandList->Reset(
            dx12::GpuCore::Instance().commandAllocator.Get(), nullptr);

        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            dx12::GpuCore::Instance()
                .renderTargets[dx12::GpuCore::Instance().frameIndex]
                .Get(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

        dx12::GpuCore::Instance().commandList->ResourceBarrier(1, &barrier);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
            dx12::GpuCore::Instance()
                .rtvHeap->GetCPUDescriptorHandleForHeapStart(),
            dx12::GpuCore::Instance().frameIndex,
            dx12::GpuCore::Instance().rtvDescriptorSize);
        const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        dx12::GpuCore::Instance().commandList->ClearRenderTargetView(
            rtvHandle, clearColor, 0, nullptr);
        dx12::GpuCore::Instance().commandList->OMSetRenderTargets(
            1, &rtvHandle, false, nullptr);

        dx12::GpuCore::Instance().commandList->SetDescriptorHeaps(
            1, dx12::GpuCore::Instance().srvHeap.GetAddressOf());

        gui->Frame();
        ImGui_ImplDX12_RenderDrawData(
            ImGui::GetDrawData(), dx12::GpuCore::Instance().commandList.Get());

        barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            dx12::GpuCore::Instance()
                .renderTargets[dx12::GpuCore::Instance().frameIndex]
                .Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        dx12::GpuCore::Instance().commandList->ResourceBarrier(1, &barrier);

        // Close the command list after all commands have been added
        dx12::GpuCore::Instance().commandList->Close();
    }

    {
        // Execute the command list
        dx12::GpuCore::Instance().command_queue->ExecuteCommandLists(
            1, CommandListCast(
                   dx12::GpuCore::Instance().commandList.GetAddressOf()));
        dx12::GpuCore::Instance().swap_chain->Present(1, 0);
        WaitForPreviousFrame();
    }

    return true;
}

bool Engine::Stop() {
    Platform::Stop();

    // if (black_board) {

    //    if (black_board->job_context) {
    //        for (auto &model : black_board->job_context->objects) {
    //            model.second.reset();
    //        }
    //        black_board->job_context->camera.reset();
    //    }

    //    if (black_board->gui) {
    //        black_board->gui->Shutdown();
    //        black_board->gui.reset();
    //    }

    //    if (black_board->input) {
    //        black_board->input.reset();
    //    }
    //}

    return true;
}

} // namespace core