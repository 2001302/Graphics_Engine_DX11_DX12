#ifndef _INFO
#define _INFO

#include "entity.h"

namespace common {

struct IInfo : public IEntity {
  public:
    virtual void Show(){};
};
} // namespace common
#endif
