#ifndef _ENV
#define _ENV

#include "common_struct.h"
#include "dataBlock.h"

namespace Engine {
class Env : public IDataBlock {
  public:
    const float screenDepth = 1000.0f;
    const float screenNear = 0.3f;
    const float fieldOfView = M_PI / 4.0f;

    float screenWidth;
    float screenHeight;
    float aspect;
};
} // namespace Engine
#endif