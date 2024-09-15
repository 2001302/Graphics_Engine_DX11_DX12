#ifndef _INFO
#define _INFO

#include "entity.h"

namespace common {

struct IInfo : public IEntity {
  public:
    void Show() { OnShow(); };

  private:
    virtual void OnShow(){};
};
} // namespace common
#endif
