#ifndef _COMPONENT
#define _COMPONENT

#include "foundation/node.h"

namespace graphics {
enum EnumComponentType {
    eRenderer = 0,
    eAnimator = 1,
};

class Component {
  public:
    Component(){};
};

} // namespace engine
#endif