#ifndef _COMPONENT
#define _COMPONENT

#include <node.h>

namespace graphics {
enum EnumComponentType {
    eRenderer = 0,
    eAnimator = 1,
};

class Component {
  public:
    Component(){};
};

} // namespace graphics
#endif
