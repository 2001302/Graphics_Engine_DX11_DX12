#pragma warning(disable : 6385)
#pragma warning(disable : 6386)

#include "game_object.h"

using namespace Engine;

GameObject::GameObject() {
    phong_shader_source = std::make_shared<PhongShaderSource>();
    phong_shader_source->Initialize();
    image_based_shader_source = std::make_shared<ImageBasedShaderSource>();
    image_based_shader_source->Initialize();
    physically_based_shader_source =
        std::make_shared<PhsicallyBasedShaderSource>();
    physically_based_shader_source->Initialize();
}

GameObject::~GameObject() {}

int GameObject::GetIndexCount() {
    int count = 0;
    for (auto mesh : meshes)
        count += mesh->indices.size();
    return count;
}
