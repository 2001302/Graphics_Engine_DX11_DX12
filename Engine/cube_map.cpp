#include "cube_map.h"

namespace engine {

CubeMap::CubeMap() {}

CubeMap::~CubeMap() {}

int CubeMap::GetIndexCount() { return mesh->indices.size(); }
} // namespace engine