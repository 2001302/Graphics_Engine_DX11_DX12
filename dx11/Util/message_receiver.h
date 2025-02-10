#ifndef _MESSAGERECEIVER
#define _MESSAGERECEIVER

#include "geometry_generator.h"
#include "../pipeline/render_target_object.h"
#include <structure/input.h>
#include <gui/setting_ui.h>

namespace graphics {
class MessageReceiver {
  public:
    bool OnModelLoadRequest(RenderTargetObject *manager, HWND main_window);
    bool OnSphereLoadRequest(RenderTargetObject *manager);
    bool OnBoxLoadRequest(RenderTargetObject *manager);
    bool OnCylinderLoadRequest(RenderTargetObject *manager);
    bool OnMouseDownRequest(std::shared_ptr<common::Input> input, int mouseX,
                            int mouseY);
    bool OnMouseRightDragRequest(RenderTargetObject *manager,
                                 std::shared_ptr<common::Input> input,
                                 int mouseX, int mouseY);
    bool OnMouseWheelRequest(RenderTargetObject *manager,
                             std::shared_ptr<common::Input> input, int wheel);
    bool OnMouseWheelDragRequest(RenderTargetObject *manager,
                                 std::shared_ptr<common::Input> input,
                                 int mouseX, int mouseY);
    bool OnWindowSizeRequest(common::SettingUi *gui, int size_x, int size_y);
};
} // namespace graphics

#endif
