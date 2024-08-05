#ifndef _MESSAGERECEIVER
#define _MESSAGERECEIVER

#include "geometry_generator.h"
#include "input.h"
#include "rendering_block.h"
#include "setting_ui.h"

namespace engine {
class MessageReceiver {
  public:
    bool OnModelLoadRequest(RenderingBlock *manager, HWND main_window);
    bool OnSphereLoadRequest(RenderingBlock *manager);
    bool OnBoxLoadRequest(RenderingBlock *manager);
    bool OnCylinderLoadRequest(RenderingBlock *manager);
    bool OnMouseDownRequest(std::shared_ptr<common::Input> input, int mouseX,
                            int mouseY);
    bool OnMouseRightDragRequest(RenderingBlock *manager,
                                 std::shared_ptr<common::Input> input,
                                 int mouseX, int mouseY);
    bool OnMouseWheelRequest(RenderingBlock *manager,
                             std::shared_ptr<common::Input> input, int wheel);
    bool OnMouseWheelDragRequest(RenderingBlock *manager,
                                 std::shared_ptr<common::Input> input,
                                 int mouseX, int mouseY);
    bool OnWindowSizeRequest(SettingUi *gui, int size_x, int size_y);
};
} // namespace engine

#endif
