#ifndef _COMPONENT
#define _COMPONENT

#include "node_ui.h"

namespace engine {
enum EnumComponentType {
    eRenderer = 0,
};

class Component : public common::INodeUi {
  public:
    Component(){};
};

} // namespace engine
#endif
