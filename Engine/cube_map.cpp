#include "cube_map.h"

using namespace Engine;

CubeMap::CubeMap() {
    cube_map_shader_source = std::make_shared<CubeMapShaderSource>();
    cube_map_shader_source->Initialize();
}

CubeMap::~CubeMap() {}

int CubeMap::GetIndexCount() {
    return mesh->indices.size();
}
