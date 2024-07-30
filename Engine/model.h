#ifndef _MODEL
#define _MODEL

#include "component.h"
#include "graphics_manager.h"
#include "node.h"
#include <unordered_map>

namespace engine {
using Microsoft::WRL::ComPtr;

class Model : public common::INode {
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
