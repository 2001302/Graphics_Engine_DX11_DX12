#ifndef _MESSAGERECEIVER
#define _MESSAGERECEIVER

#include "input.h"
#include "geometry_generator.h"
#include "pipeline_manager.h"
#include "resource_helper.h"
#include "game_object_node.h"

namespace Engine {
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
};
} // namespace Engine

#endif
