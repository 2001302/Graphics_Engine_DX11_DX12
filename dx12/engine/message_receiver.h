#ifndef _MESSAGERECEIVER
#define _MESSAGERECEIVER

#include "geometry_generator.h"
#include "input.h"
#include "job_context.h"
#include "../foundation/setting_ui.h"

namespace core {
class MessageReceiver {
  public:
    bool OnModelLoadRequest(JobContext *manager, HWND main_window);
    bool OnSphereLoadRequest(JobContext *manager);
    bool OnBoxLoadRequest(JobContext *manager);
    bool OnCylinderLoadRequest(JobContext *manager);
    bool OnMouseDownRequest(std::shared_ptr<foundation::Input> input, int mouseX,
                            int mouseY);
    bool OnMouseRightDragRequest(JobContext *manager,
                                 std::shared_ptr<foundation::Input> input,
                                 int mouseX, int mouseY);
    bool OnMouseWheelRequest(JobContext *manager,
                             std::shared_ptr<foundation::Input> input, int wheel);
    bool OnMouseWheelDragRequest(JobContext *manager,
                                 std::shared_ptr<foundation::Input> input,
                                 int mouseX, int mouseY);
    bool OnWindowSizeRequest(foundation::SettingUi *gui, int size_x, int size_y);
};
} // namespace engine

#endif
