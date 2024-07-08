#include "cube_map.h"

using namespace Engine;

CubeMap::CubeMap() {
    cube_map_shader_source = std::make_shared<CubeMapShaderSource>();
}

CubeMap::~CubeMap() {}

int CubeMap::GetIndexCount() {
    return mesh->indices.size();
}
