#ifndef _ENTITY
#define _ENTITY

#include "common_struct.h"
#include "phong_shader.h"

namespace Engine {
using Microsoft::WRL::ComPtr;

class IEntity {
  public:
    IEntity();
    ~IEntity();

    int entity_id_;
};
} // namespace Engine
#endif