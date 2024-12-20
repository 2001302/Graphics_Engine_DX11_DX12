#ifndef _OBJECT_POOL
#define _OBJECT_POOL

#include "data_set.h"
#include "model.h"

namespace common {
class ObjectPool {
  public:
    ObjectPool(){};
    ~ObjectPool(){};

  private:
    std::shared_ptr<std::vector<Model>> objects;
};
} // namespace common

#endif