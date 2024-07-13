#ifndef _MODEL
#define _MODEL

#include "common_struct.h"
#include "game_object.h"
#include "image_based_shader.h"
#include "phong_shader.h"
#include "physically_based_shader.h"

namespace dx11 {
using Microsoft::WRL::ComPtr;

class Model : public GameObject{
  public:
    Model();
    ~Model();

    int GetIndexCount();

    Vector3 translation = Vector3(0.0f);
    Vector3 rotation = Vector3(0.0f);
    Vector3 scaling = Vector3(1.0f);

    std::vector<std::shared_ptr<Bone>> bones;
    std::vector<std::shared_ptr<Mesh>> meshes;
    // std::shared_ptr<Animation> animation;

  private:
    void OnShow() override;
};

} // namespace Engine
#endif
