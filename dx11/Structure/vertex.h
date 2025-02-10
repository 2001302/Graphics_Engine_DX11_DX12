#ifndef _VERTEX
#define _VERTEX

#include "../gpu/graphics_util.h"

namespace graphics {

struct Vertex {
    Vector3 position;
    Vector3 normal;
    Vector2 texcoord;
    Vector3 tangent;
};

struct SkinnedVertex {
    Vector3 position;
    Vector3 normalModel;
    Vector2 texcoord;
    Vector3 tangentModel;

    float blendWeights[8] = {0.0f, 0.0f, 0.0f, 0.0f,
                             0.0f, 0.0f, 0.0f, 0.0f};  // BLENDWEIGHT0 and 1
    uint8_t boneIndices[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // BLENDINDICES0 and 1
};

} // namespace engine
#endif
