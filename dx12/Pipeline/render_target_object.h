#ifndef _RENDER_TARGET_OBJECT
#define _RENDER_TARGET_OBJECT

#include "camera.h"
#include <structure/info.h>

namespace graphics {
class RenderTargetObject : public common::IInfo {
  public:
    RenderTargetObject() {
        camera = std::make_unique<Camera>();
        world = std::make_shared<common::Model>();
    }
    std::unique_ptr<Camera> camera;
    std::shared_ptr<common::Model> world; // shadow,skybox,mirror ground
    std::map<int /*id*/, std::shared_ptr<common::Model>> objects;
};
} // namespace graphics
#endif
