#include "cube_map.h"

using namespace Engine;

CubeMap::CubeMap() {}

CubeMap::~CubeMap() {}


int CubeMap::GetIndexCount() {
    int count = 0;
    for (auto mesh : meshes)
        count += mesh->indices.size();
    return count;
}
