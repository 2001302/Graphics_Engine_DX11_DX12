#ifndef _MODEL
#define _MODEL

#include "component.h"
#include "node.h"
#include <memory>
#include <typeinfo>
#include <unordered_map>

namespace common {
class Model : public INode {
  public:
    Model(){};
    ~Model();

    template <typename T> bool TryAdd(std::shared_ptr<T> component) {
        if (components.contains(typeid(T).name()))
            return false;
        else {
            components[typeid(T).name()] = component;
            return true;
        }
    }
    template <typename T> bool TryGet(T *&component) {
        if (!components.contains(typeid(T).name()))
            return false;

        component = (T*)components[typeid(T).name()].get();
        if (component == nullptr)
            return false;
        else
            return true;
    }
    template <typename T> bool Has() {
        if (components.contains(typeid(T).name()))
            return true;
        else 
            return true;
    }

  private:
    std::unordered_map<const char *, std::shared_ptr<Component>> components;
};
} // namespace common
#endif
