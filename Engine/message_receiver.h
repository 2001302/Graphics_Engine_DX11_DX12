#ifndef _MESSAGERECEIVER
#define _MESSAGERECEIVER

#include "geometry_generator.h"
#include "input.h"
#include "pipeline_manager.h"

namespace dx11 {
class MessageReceiver {
  public:
    bool OnModelLoadRequest(PipelineManager *manager, HWND main_window);
    bool OnSphereLoadRequest(PipelineManager *manager);
    bool OnBoxLoadRequest(PipelineManager *manager);
    bool OnCylinderLoadRequest(PipelineManager *manager);

    bool OnRightDragRequest(PipelineManager *manager,
                            std::shared_ptr<Input> input);
    bool OnMouseWheelRequest(PipelineManager *manager,
                             std::shared_ptr<Input> input);
    bool OnWheelDragRequest(PipelineManager *manager,
                            std::shared_ptr<Input> input, int mouseX,
                            int mouseY);
};
} // namespace dx11

#endif
