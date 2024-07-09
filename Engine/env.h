#ifndef _ENV
#define _ENV

#include "common_struct.h"
#include "dataBlock.h"

namespace Engine {
struct Env {
  private:
    Env() : screen_width(1920), screen_height(1080), aspect(0.0f){};
  public:
    static Env &Get() {
        static Env instance;
        return instance;
    }

    const float field_of_view = M_PI / 4.0f;
    const float screen_depth = 1000.0f;
    const float screen_near = 0.3f;
    float screen_width;
    float screen_height;
    float aspect;
};
} // namespace Engine
#endif
