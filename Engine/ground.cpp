#include "ground.h"

using namespace Engine;

Ground::Ground() {
}

Ground::~Ground() {}

int Ground::GetIndexCount() {
    return mesh->indices.size();
}
