#ifndef _COMPONENT
#define _COMPONENT

#include "node.h"

namespace engine {
enum EnumComponentType {
    eRenderer = 0,
    eAnimator = 1,
};

class Component : public common::INode {
  public:
    Component(){};
};

} // namespace engine
#endif
