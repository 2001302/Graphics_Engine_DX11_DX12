#ifndef _RENDER_TARGET_OBJECT
#define _RENDER_TARGET_OBJECT

#include "camera.h"
#include <info.h>

namespace graphics {
class RenderTargetObject : public common::IInfo {
  public:
    RenderTargetObject() {}
    std::unique_ptr<Camera> camera;
    std::shared_ptr<common::Model> world;
    std::shared_ptr<common::Model> ground;
    std::map<int /*id*/, std::shared_ptr<common::Model>> objects;
};
} // namespace graphics
#endif
