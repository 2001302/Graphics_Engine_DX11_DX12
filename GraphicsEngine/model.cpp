#include "model.h"

namespace engine {
Model::~Model() {
    for (auto component : components) {
        component.second.reset();
    }
}
void Model::AddComponent(EnumComponentType type,
                         std::shared_ptr<Component>component) {
    if (components.find(type) == components.end()) {
        components.insert({type, component}); 
    } 
}

Component*
Model::GetComponent(EnumComponentType type) {
    auto it = components.find(type);
    if (it != components.end()) {
        return it->second.get();
    } else {
        return nullptr;
    }
};
}