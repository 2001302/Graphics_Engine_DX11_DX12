#ifndef _MESSAGERECEIVER
#define _MESSAGERECEIVER

#include "geometry_generator.h"
#include "input.h"
#include "rendering_block.h"

namespace engine {
class MessageReceiver {
  public:
    bool OnModelLoadRequest(RenderingBlock *manager, HWND main_window);
    bool OnSphereLoadRequest(RenderingBlock *manager);
    bool OnBoxLoadRequest(RenderingBlock *manager);
    bool OnCylinderLoadRequest(RenderingBlock *manager);

    bool OnMouseRightDragRequest(RenderingBlock *manager,
                            std::shared_ptr<Input> input);
    bool OnMouseWheelRequest(RenderingBlock *manager,
                             std::shared_ptr<Input> input);
    bool OnMouseWheelDragRequest(RenderingBlock *manager,
                            std::shared_ptr<Input> input, int mouseX,
                            int mouseY);
};
} // namespace dx11

#endif
