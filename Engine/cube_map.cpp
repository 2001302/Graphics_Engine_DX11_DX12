#include "cube_map.h"

using namespace Engine;

CubeMap::CubeMap() {
}

CubeMap::~CubeMap() {}

int CubeMap::GetIndexCount() {
    return mesh->indices.size();
}
