#ifndef _APPLICATION
#define _APPLICATION

#include "black_board.h"
#include "foundation/behavior_tree_builder.h"
#include "foundation/platform.h"
#include "foundation/setting_ui.h"
#include "graphics/graphics_util.h"
#include "message_receiver.h"

namespace core {

class Engine : public foundation::Platform {
  public:
    Engine();

    bool Start() override final;
    bool Frame() override final;
    bool Stop() override final;

  private:
    std::shared_ptr<BlackBoard> black_board;
    std::unique_ptr<MessageReceiver> message_receiver;

    std::shared_ptr<foundation::BehaviorTreeBuilder> start_tree;
    std::shared_ptr<foundation::BehaviorTreeBuilder> update_tree;
    std::shared_ptr<foundation::BehaviorTreeBuilder> render_tree;

    LRESULT CALLBACK MessageHandler(HWND main_window, UINT umsg, WPARAM wparam,
                                    LPARAM lparam) {

        auto CheckIfMouseInViewport = [](foundation::SettingUi *ui, int mouseX,
                                         int mouseY) -> bool {
            if (dx12::GpuCore::Instance().swap_chain) {
                if ((0 < mouseX && mouseX < ui->GetSize().x) &&
                    (0 < mouseY && mouseY < ui->GetSize().y))
                    return false;
            }
            return true;
        };

        extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
            HWND main_window, UINT msg, WPARAM wParam, LPARAM lParam);

        if (ImGui_ImplWin32_WndProcHandler(main_window, umsg, wparam, lparam))
            return true;

        switch (umsg) {
        case WM_SIZE: {
            return message_receiver->OnWindowSizeRequest(black_board->gui.get(),
                                                         int(LOWORD(lparam)),
                                                         int(HIWORD(lparam)));
            break;
        }
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
                Stop();
                DestroyWindow(main_window);
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
} // namespace core
#endif
