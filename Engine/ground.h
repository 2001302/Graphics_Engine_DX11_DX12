#ifndef _GROUND
#define _GROUND

#include "common_struct.h"
#include "game_object.h"
#include "ground_shader.h"

namespace Engine {
using Microsoft::WRL::ComPtr;

class Ground : public GameObject {
  public:
    Ground();
    ~Ground();

    int GetIndexCount();

    DirectX::SimpleMath::Matrix transform;
    std::shared_ptr<Mesh> mesh;
};
} // namespace Engine
#endif