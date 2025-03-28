#ifndef _APPLICATION
#define _APPLICATION

#include "pipeline/black_board.h"
#include "util/mesh_util.h"
#include "util/message_receiver.h"
#include "tree_node.h"
#include <behaviortree/behavior_tree_builder.h>
#include <platform/platform.h>
#include <gui/setting_ui.h>
#include <util/time_stamp.h>

namespace graphics {

class Engine : public common::Platform {
  public:
    Engine();

    bool Start() override final;
    bool Frame() override final;
    bool Stop() override final;

    virtual void LoadAsset(BlackBoard *black_board){};

  private:
    std::shared_ptr<BlackBoard> black_board;
    std::unique_ptr<MessageReceiver> message_receiver;
    std::unique_ptr<common::TimeStamp> time_stamp;

    std::shared_ptr<common::BehaviorTreeBuilder> start_tree;
    std::shared_ptr<common::BehaviorTreeBuilder> update_tree;
    std::shared_ptr<common::BehaviorTreeBuilder> render_tree;

    LRESULT CALLBACK MessageHandler(HWND main_window, UINT umsg, WPARAM wparam,
                                    LPARAM lparam) {

        auto CheckIfMouseInViewport = [](common::SettingUi *ui, int mouseX,
                                         int mouseY) -> bool {
            if (GpuCore::Instance().GetSwapChain()) {
                if (ui->GetViewType() == common::SettingUi::EnumViewType::eGame)
                    return false;

                common::Rect rect = ui->GetRect();

                if ((rect.left < mouseX && mouseX < rect.right) &&
                    (rect.top < mouseY && mouseY < rect.bottom))
                    return true;
                else
                    return false;
            }
            return true;
        };

        extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
            HWND main_window, UINT msg, WPARAM wParam, LPARAM lParam);

        if (ImGui_ImplWin32_WndProcHandler(main_window, umsg, wparam, lparam))
            return true;

        switch (umsg) {
        case WM_MOUSEMOVE: {
            if (wparam & MK_RBUTTON) {
                if (CheckIfMouseInViewport(black_board->gui.get(),
                                           LOWORD(lparam), HIWORD(lparam))) {
                    return message_receiver->OnMouseRightDragRequest(
                        black_board->targets.get(), black_board->input,
                        LOWORD(lparam), HIWORD(lparam));
                }
            }
            if (wparam & MK_MBUTTON) {
                if (CheckIfMouseInViewport(black_board->gui.get(),
                                           LOWORD(lparam), HIWORD(lparam))) {
                    return message_receiver->OnMouseWheelDragRequest(
                        black_board->targets.get(), black_board->input,
                        LOWORD(lparam), HIWORD(lparam));
                }
            }
            break;
        }
        case WM_MOUSEWHEEL: {
            if (CheckIfMouseInViewport(black_board->gui.get(), LOWORD(lparam),
                                       HIWORD(lparam))) {
                return message_receiver->OnMouseWheelRequest(
                    black_board->targets.get(), black_board->input,
                    GET_WHEEL_DELTA_WPARAM(wparam));
            }
            break;
        }
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN: {
            return message_receiver->OnMouseDownRequest(
                black_board->input, LOWORD(lparam), HIWORD(lparam));
            break;
        }
        case WM_MODEL_LOAD: {
            return message_receiver->OnModelLoadRequest(
                black_board->targets.get(), main_window);
            break;
        }
        case WM_SPHERE_LOAD: {
            return message_receiver->OnSphereLoadRequest(
                black_board->targets.get());
            break;
        }
        case WM_BOX_LOAD: {
            return message_receiver->OnBoxLoadRequest(
                black_board->targets.get());
            break;
        }
        case WM_CYLINDER_LOAD: {
            return message_receiver->OnCylinderLoadRequest(
                black_board->targets.get());
            break;
        }
        case WM_KEYDOWN:
            black_board->input->KeyPressed(wparam, true);
            if (wparam == VK_ESCAPE) {
                //DestroyWindow(main_window);
                //::PostQuitMessage(0);
            }
            break;
        case WM_KEYUP:
            black_board->input->KeyPressed(wparam, false);
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
        default: {
            return DefWindowProc(main_window, umsg, wparam, lparam);
        }
        }
    }
};
} // namespace graphics
#endif
