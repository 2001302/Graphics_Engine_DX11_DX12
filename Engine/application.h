#ifndef _APPLICATION
#define _APPLICATION

#include "action_node.h"
#include "geometry_generator.h"
#include "graphics_manager.h"
#include "input.h"
#include "message.h"
#include "message_receiver.h"
#include "platform.h"
#include "rendering_block.h"
#include "setting_ui.h"
#include "tree_node.h"

namespace engine {
class Application : public Platform, TreeNode {
  public:
    Application();

    bool OnStart() override final;
    bool OnFrame() override final;
    bool OnStop() override final;

  private:
    bool OnUpdate(float dt);
    bool OnRender();

    std::shared_ptr<RenderingBlock> render_block;
    std::unique_ptr<MessageReceiver> message_receiver;
    std::shared_ptr<Input> input;
    std::shared_ptr<common::SettingUi> gui;

    LRESULT CALLBACK MessageHandler(HWND main_window, UINT umsg, WPARAM wparam,
                                    LPARAM lparam) {

        auto CheckIfMouseInViewport = [](common::SettingUi *ui, int mouseX,
                                         int mouseY) -> bool {
            if (GraphicsManager::Instance().swap_chain) {
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

            common::Env::Instance().screen_width = int(LOWORD(lparam));
            common::Env::Instance().screen_height = int(HIWORD(lparam));

            if (common::Env::Instance().screen_width &&
                common::Env::Instance().screen_height) {
                if (GraphicsManager::Instance().swap_chain) {

                    gui->Shutdown();

                    GraphicsManager::Instance().back_buffer_RTV.Reset();
                    GraphicsManager::Instance().swap_chain->ResizeBuffers(
                        0, (UINT)LOWORD(lparam), (UINT)HIWORD(lparam),
                        DXGI_FORMAT_UNKNOWN, 0);

                    GraphicsManager::Instance().CreateBuffer();
                    GraphicsManager::Instance().SetMainViewport();

                    gui->Initialize();
                }
            }
            break;
        }
        case WM_MOUSEMOVE: {
            if (wparam & MK_RBUTTON) {
                if (CheckIfMouseInViewport(gui.get(), LOWORD(lparam),
                                           HIWORD(lparam))) {
                    return message_receiver->OnMouseRightDragRequest(
                        render_block.get(), input);
                }
            }
            if (wparam & MK_MBUTTON) {
                if (CheckIfMouseInViewport(gui.get(), LOWORD(lparam),
                                           HIWORD(lparam))) {
                    return message_receiver->OnMouseWheelDragRequest(
                        render_block.get(), input, LOWORD(lparam), HIWORD(lparam));
                }
            }
            break;
        }
        case WM_MOUSEWHEEL: {
            if (CheckIfMouseInViewport(gui.get(), LOWORD(lparam),
                                       HIWORD(lparam))) {
                return message_receiver->OnMouseWheelRequest(render_block.get(),
                                                              input);
            }
            break;
        }
        case WM_RBUTTONDOWN: {
            break;
        }
        case WM_LBUTTONUP: {
            break;
        }
        case WM_MODEL_LOAD: {
            return message_receiver->OnModelLoadRequest(render_block.get(),
                                                         main_window);
            break;
        }
        case WM_SPHERE_LOAD: {
            return message_receiver->OnSphereLoadRequest(render_block.get());
            break;
        }
        case WM_BOX_LOAD: {
            return message_receiver->OnBoxLoadRequest(render_block.get());
            break;
        }
        case WM_CYLINDER_LOAD: {
            return message_receiver->OnCylinderLoadRequest(render_block.get());
            break;
        }
        case WM_KEYDOWN:
            if (wparam == VK_ESCAPE) {
                OnStop();
                DestroyWindow(main_window);
            }
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
} // namespace engine
#endif
