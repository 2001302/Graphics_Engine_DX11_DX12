#ifndef _MESSAGERECEIVER
#define _MESSAGERECEIVER

#include "foundation/input.h"
#include "foundation/setting_ui.h"
#include "geometry_generator.h"
#include "render_target_object.h"

namespace graphics {
class MessageReceiver {
  public:
    bool OnModelLoadRequest(RenderTargetObject *manager, HWND main_window);
    bool OnSphereLoadRequest(RenderTargetObject *manager);
    bool OnBoxLoadRequest(RenderTargetObject *manager);
    bool OnCylinderLoadRequest(RenderTargetObject *manager);
    bool OnMouseDownRequest(std::shared_ptr<foundation::Input> input,
                            int mouseX, int mouseY);
    bool OnMouseRightDragRequest(RenderTargetObject *manager,
                                 std::shared_ptr<foundation::Input> input,
                                 int mouseX, int mouseY);
    bool OnMouseWheelRequest(RenderTargetObject *manager,
                             std::shared_ptr<foundation::Input> input,
                             int wheel);
    bool OnMouseWheelDragRequest(RenderTargetObject *manager,
                                 std::shared_ptr<foundation::Input> input,
                                 int mouseX, int mouseY);
    bool OnWindowSizeRequest(foundation::SettingUi *gui, int size_x,
                             int size_y);
};
} // namespace core

#endif
