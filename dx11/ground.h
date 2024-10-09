#ifndef _GROUND
#define _GROUND

#include <model.h>

namespace graphics {
struct Ground {
    std::shared_ptr<common::Model> model;
    std::shared_ptr<common::Model> mirror;
    DirectX::SimpleMath::Plane mirror_plane;
    float mirror_alpha = 1.0f; // opacity
};
} // namespace engine
#endif
