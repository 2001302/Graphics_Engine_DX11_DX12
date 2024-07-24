#ifndef _MODEL
#define _MODEL

#include "component.h"
#include "graphics_manager.h"
#include "node_ui.h"
#include <unordered_map>

namespace engine {
using Microsoft::WRL::ComPtr;

class Model : public common::INodeUi {
  public:
    Model(){};
    bool AddComponent(EnumComponentType type, Component *component) {
        if (components.find(type) == components.end()) {
            components.insert({type, component});
            return true;
        } else
            return false;
    }

    bool GetComponent(EnumComponentType type, OUT Component** component) {
        auto it = components.find(type);
        if (it != components.end()) {
            *component = it->second;
            return true;
        } else {
            *component = nullptr;
            return false;
        }
    };

  private:
    std::unordered_map<EnumComponentType, Component *> components;
};
} // namespace engine
#endif
