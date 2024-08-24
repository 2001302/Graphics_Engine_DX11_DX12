#ifndef _MESSAGERECEIVER
#define _MESSAGERECEIVER

#include "geometry_generator.h"
#include "input.h"
#include "job_context.h"
#include "../foundation/setting_ui.h"

namespace core {
class MessageReceiver {
  public:
    bool OnModelLoadRequest(RenderTarget *manager, HWND main_window);
    bool OnSphereLoadRequest(RenderTarget *manager);
    bool OnBoxLoadRequest(RenderTarget *manager);
    bool OnCylinderLoadRequest(RenderTarget *manager);
    bool OnMouseDownRequest(std::shared_ptr<foundation::Input> input, int mouseX,
                            int mouseY);
    bool OnMouseRightDragRequest(RenderTarget *manager,
                                 std::shared_ptr<foundation::Input> input,
                                 int mouseX, int mouseY);
    bool OnMouseWheelRequest(RenderTarget *manager,
                             std::shared_ptr<foundation::Input> input, int wheel);
    bool OnMouseWheelDragRequest(RenderTarget *manager,
                                 std::shared_ptr<foundation::Input> input,
                                 int mouseX, int mouseY);
    bool OnWindowSizeRequest(foundation::SettingUi *gui, int size_x, int size_y);
};
} // namespace engine

#endif
