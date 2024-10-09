#ifndef _COMPONENT
#define _COMPONENT

#include "node.h"

namespace common {
enum EnumComponentType {
    eRenderer = 0,
    eAnimator = 1,
};

class Component {
  public:
    Component(){};
};

} // namespace common
#endif
