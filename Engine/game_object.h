#ifndef _GAMEOBJECT
#define _GAMEOBJECT

#include "common_struct.h"
#include "entity.h"
#include "image_based_shader.h"
#include "phong_shader.h"
#include "physically_based_shader.h"

namespace Engine {
using Microsoft::WRL::ComPtr;

class GameObject : public IEntity {
  public:
    GameObject(){};
    ~GameObject(){};
    void Update(){};
    void Render(){};

  private:
    virtual void OnUpdate(){};
    virtual void OnUpdateGui(){};
    virtual void OnRender(){};
    DirectX::SimpleMath::Matrix transform;
    std::vector<std::shared_ptr<Mesh>> meshes;
};
} // namespace Engine
#endif