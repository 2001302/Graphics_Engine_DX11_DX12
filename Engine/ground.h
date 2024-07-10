#ifndef _GROUND
#define _GROUND

#include "common_struct.h"
#include "entity.h"
#include "game_object.h"
#include "phong_shader.h"

namespace Engine {
using Microsoft::WRL::ComPtr;

class Ground : public GameObject {
  public:
    Ground();
    ~Ground();

    int GetIndexCount();

    DirectX::SimpleMath::Matrix transform;
    std::vector<std::shared_ptr<Mesh>> meshes;

    std::shared_ptr<PhongShaderSource> phong_shader_source;
};
} // namespace Engine
#endif