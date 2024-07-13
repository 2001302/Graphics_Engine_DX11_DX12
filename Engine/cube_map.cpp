#include "cube_map.h"

using namespace dx11;

CubeMap::CubeMap() {
}

CubeMap::~CubeMap() {}

int CubeMap::GetIndexCount() {
    return mesh->indices.size();
}
