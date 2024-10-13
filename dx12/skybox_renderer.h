#ifndef _SKYBOX_RENDERER
#define _SKYBOX_RENDERER

#include "mesh_renderer.h"
namespace graphics {
class SkyboxRenderer : public MeshRenderer {
  public:
    SkyboxRenderer(){};
    ~SkyboxRenderer(){};
};
} // namespace graphics
#endif