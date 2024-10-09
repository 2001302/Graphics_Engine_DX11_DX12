#include "model.h"

namespace common {
Model::~Model() {
    for (auto component : components) {
        component.second.reset();
    }
}
} // namespace graphics
