#ifndef _MODEL
#define _MODEL

#include "component.h"
#include "graphics_manager.h"
#include "node_ui.h"
#include <unordered_map>

namespace engine {
using Microsoft::WRL::ComPtr;

class Model : public common::INode {
  public:
    Model(){};
    ~Model();
    bool AddComponent(EnumComponentType type, Component *component);
    bool GetComponent(EnumComponentType type, OUT Component **component);

  private:
    std::unordered_map<EnumComponentType, Component *> components;
};
} // namespace engine
#endif
