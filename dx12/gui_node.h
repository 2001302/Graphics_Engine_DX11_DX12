#ifndef _GUINODE
#define _GUINODE

#include "black_board.h"
#include "foundation/behavior_tree_builder.h"

namespace graphics {

class GuiNodeInvoker : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto target = black_board->targets.get();
        auto condition = black_board->conditions.get();

        auto gui = black_board->gui;

        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {
            gui->Start();
            gui->PushInfoItem(target);

            ImGui_ImplWin32_Init(foundation::Env::Instance().main_window);
            ImGui_ImplDX12_Init(
                GpuCore::Instance().GetDevice(), SWAP_CHAIN_BUFFER_COUNT,
                DXGI_FORMAT_R8G8B8A8_UNORM,
                &GpuCore::Instance().GetHeap().View()->Get(),
                GpuCore::Instance().GetHeap().View()->GetCpuHandle(0),
                GpuCore::Instance().GetHeap().View()->GetGpuHandle(0));

            break;
        }
        case EnumStageType::eRender: {
            gui->Frame();
            gui->ClearNodeItem();

            auto command_manager = GpuCore::Instance().GetCommand();

            command_manager.GraphicsList()->TransitionResource(
                GpuCore::Instance().GetDisplay(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, true);

            command_manager.GraphicsList()->SetViewportAndScissorRect(
                0, 0, (UINT)foundation::Env::Instance().screen_width,
                (UINT)foundation::Env::Instance().screen_height);

            command_manager.GraphicsList()->SetRenderTargetView(
                GpuCore::Instance().GetDisplay());

            ImGui_ImplDX12_RenderDrawData(
                ImGui::GetDrawData(),
                command_manager.GraphicsList()->GetList());

            break;
        }
        default:
            break;
        }

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace graphics

#endif
