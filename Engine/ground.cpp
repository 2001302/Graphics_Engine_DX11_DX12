#include "ground.h"

using namespace Engine;

Ground::Ground() {
    ground_shader_source = std::make_shared<GroundShaderSource>();
    ground_shader_source->Initialize();
}

Ground::~Ground() {}

int Ground::GetIndexCount() {
    return mesh->indices.size();
}
