#ifndef _GUINODE
#define _GUINODE

#include "black_board.h"
#include "common/behavior_tree_builder.h"

namespace graphics {

class GuiNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto target = black_board->targets.get();
        auto condition = black_board->conditions.get();

        auto gui = black_board->gui;

        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {
            gui->Start();
            gui->PushInfoItem(target);

            UINT index = 0;
            D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
            GpuCore::Instance().GetHeap().View()->AllocateDescriptor(cpu_handle,
                                                                     index);

            ImGui_ImplWin32_Init(common::env::main_window);
            ImGui_ImplDX12_Init(
                GpuCore::Instance().GetDevice(), SWAP_CHAIN_BUFFER_COUNT,
                DXGI_FORMAT_R8G8B8A8_UNORM,
                &GpuCore::Instance().GetHeap().View()->Get(), cpu_handle,
                GpuCore::Instance().GetHeap().View()->GetGpuHandle(index));

            break;
        }
        case EnumStageType::eRender: {
            gui->Frame();
            gui->ClearNodeItem();

            auto context =
                GpuCore::Instance().GetCommand()->Begin<GraphicsCommandContext>(
                    L"GuiNode");

            context->TransitionResource(GpuBuffer::Instance().GetDisplay(),
                                        D3D12_RESOURCE_STATE_RENDER_TARGET,
                                        true);

            context->SetDescriptorHeaps(
                std::vector{GpuCore::Instance().GetHeap().View(),
                            GpuCore::Instance().GetHeap().Sampler()});

            context->SetViewportAndScissorRect(
                0, 0, (UINT)common::env::screen_width,
                (UINT)common::env::screen_height);

            context->SetRenderTargetView(GpuBuffer::Instance().GetDisplay());

            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(),
                                          context->GetList());

            GpuCore::Instance().GetCommand()->Finish(context);

            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace graphics

#endif
