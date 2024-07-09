#ifndef _ENV
#define _ENV

#include "common_struct.h"
#include "dataBlock.h"

namespace Engine {
class Env : public IDataBlock {
  public:
    const float screen_depth_ = 1000.0f;
    const float screen_near_ = 0.3f;
    const float field_of_view_ = M_PI / 4.0f;

    float screen_width_;
    float screen_height_;
    float aspect_;
};
} // namespace Engine
#endif
