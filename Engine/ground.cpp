#include "ground.h"

using namespace dx11;

Ground::Ground() {
}

Ground::~Ground() {}

int Ground::GetIndexCount() {
    return mesh->indices.size();
}
