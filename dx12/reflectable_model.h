#ifndef _REFLECTABLE_MODEL
#define _REFLECTABLE_MODEL

#include "model.h"

namespace graphics {
struct ReflectableModel {
    std::shared_ptr<Model> mirror;
    DirectX::SimpleMath::Plane mirror_plane;
    float mirror_alpha = 0.01f; // opacity
};
} // namespace graphics
#endif
