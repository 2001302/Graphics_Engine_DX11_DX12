#ifndef _GUINODE
#define _GUINODE

#include "black_board.h"
#include <behaviortree/behavior_tree_builder.h>

namespace graphics {

class GuiNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->targets.get();
        auto gui = black_board->gui;

        switch (manager->stage_type) {
        case EnumStageType::eInitialize: {
            gui->Start();

            ImGui_ImplWin32_Init(common::env::main_window);
            ImGui_ImplDX11_Init(GpuCore::Instance().device.Get(),
                                GpuCore::Instance().device_context.Get());

            gui->PushInfoItem(manager);
            break;
        }
        case EnumStageType::eRender: {

            ImGui_ImplWin32_NewFrame();
            ImGui_ImplDX11_NewFrame();

            gui->Frame();
            gui->ClearNodeItem();

            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }

  public:
    static void Shutdown() {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
    }
};

} // namespace graphics

#endif