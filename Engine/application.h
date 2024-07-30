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

namespace engine {
class Application : public Platform {
  public:
    Application();

    bool OnStart() override final;
    bool OnFrame() override final;
    bool OnStop() override final;

  private:
    bool OnUpdate(float dt);
    bool OnRender();

    std::shared_ptr<RenderingBlock> manager_;
    std::unique_ptr<MessageReceiver> message_receiver_;
    std::shared_ptr<Input> input_;
    std::shared_ptr<common::SettingUi> imgui_;

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

                    imgui_->Shutdown();

                    GraphicsManager::Instance().back_buffer_RTV.Reset();
                    GraphicsManager::Instance().swap_chain->ResizeBuffers(
                        0, (UINT)LOWORD(lparam), (UINT)HIWORD(lparam),
                        DXGI_FORMAT_UNKNOWN, 0);

                    GraphicsManager::Instance().CreateBuffer();
                    GraphicsManager::Instance().SetMainViewport();

                    imgui_->Initialize();
                }
            }
            break;
        }
        case WM_MOUSEMOVE: {
            if (wparam & MK_RBUTTON) {
                if (CheckIfMouseInViewport(imgui_.get(), LOWORD(lparam),
                                           HIWORD(lparam))) {
                    return message_receiver_->OnMouseRightDragRequest(
                        manager_.get(), input_);
                }
            }
            if (wparam & MK_MBUTTON) {
                if (CheckIfMouseInViewport(imgui_.get(), LOWORD(lparam),
                                           HIWORD(lparam))) {
                    return message_receiver_->OnMouseWheelDragRequest(
                        manager_.get(), input_, LOWORD(lparam), HIWORD(lparam));
                }
            }
            break;
        }
        case WM_MOUSEWHEEL: {
            if (CheckIfMouseInViewport(imgui_.get(), LOWORD(lparam),
                                       HIWORD(lparam))) {
                return message_receiver_->OnMouseWheelRequest(manager_.get(),
                                                              input_);
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
            return message_receiver_->OnModelLoadRequest(manager_.get(),
                                                         main_window);
            break;
        }
        case WM_SPHERE_LOAD: {
            return message_receiver_->OnSphereLoadRequest(manager_.get());
            break;
        }
        case WM_BOX_LOAD: {
            return message_receiver_->OnBoxLoadRequest(manager_.get());
            break;
        }
        case WM_CYLINDER_LOAD: {
            return message_receiver_->OnCylinderLoadRequest(manager_.get());
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
