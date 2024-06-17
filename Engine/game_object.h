#ifndef _GAMEOBJECT
#define _GAMEOBJECT

#include "common_struct.h"
#include "phong_shader.h"
#include "entity.h"

namespace Engine {
using Microsoft::WRL::ComPtr;

class GameObject : public IEntity {
  public:
    GameObject();
    ~GameObject();

    int GetIndexCount();

    DirectX::SimpleMath::Matrix transform;

    std::vector<std::shared_ptr<Bone>> bones;
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::shared_ptr<Animation> animation;

    std::shared_ptr<PhongShaderSource> phongShader;
};
} // namespace Engine
#endif