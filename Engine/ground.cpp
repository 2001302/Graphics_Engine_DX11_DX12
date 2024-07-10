#include "ground.h"

using namespace Engine;

Ground::Ground() {
    phong_shader_source = std::make_shared<PhongShaderSource>();
    phong_shader_source->Initialize();
}

Ground::~Ground() {}

int Ground::GetIndexCount() {
    int count = 0;
    for (auto mesh : meshes)
        count += mesh->indices.size();
    return count;
}
