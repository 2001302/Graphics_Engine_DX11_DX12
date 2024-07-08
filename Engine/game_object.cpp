#pragma warning(disable : 6385)
#pragma warning(disable : 6386)

#include "game_object.h"

using namespace Engine;

GameObject::GameObject() {

    phong_shader_source = std::make_shared<PhongShaderSource>();
    image_based_shader_source =
        std::make_shared<ImageBasedShaderSource>();
    physically_based_shader_source =
        std::make_shared<PhsicallyBasedShaderSource>();
}

GameObject::~GameObject() {}

int GameObject::GetIndexCount() {
    int count = 0;
    for (auto mesh : meshes)
        count += mesh->indices.size();
    return count;
}
