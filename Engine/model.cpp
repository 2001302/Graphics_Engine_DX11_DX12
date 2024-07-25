#include "model.h"

namespace engine {
Model::~Model() {
    for (auto component : components) {
        delete component.second;
        component.second = 0;
    }
}
bool Model::AddComponent(EnumComponentType type, Component *component) {
    if (components.find(type) == components.end()) {
        components.insert({type, component});
        return true;
    } else
        return false;
}

bool Model::GetComponent(EnumComponentType type, OUT Component **component) {
    auto it = components.find(type);
    if (it != components.end()) {
        *component = it->second;
        return true;
    } else {
        *component = nullptr;
        return false;
    }
};
}