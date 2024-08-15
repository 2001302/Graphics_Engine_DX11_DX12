#ifndef _MODEL
#define _MODEL

#include "component.h"
#include "graphics_util.h"
#include "node.h"
#include <unordered_map>

namespace core {
class Model : public foundation::INode {
  public:
    Model(){};
    ~Model();
    void AddComponent(EnumComponentType type,
                      std::shared_ptr<Component> component);
    Component *GetComponent(EnumComponentType type);

  private:
    std::unordered_map<EnumComponentType, std::shared_ptr<Component>>
        components;
};
} // namespace engine
#endif
