#ifndef _MODEL
#define _MODEL

#include "common_struct.h"
#include "phong_shader.h"
#include "image_based_shader.h"
#include "physically_based_shader.h"
#include "entity.h"
#include "game_object.h"

namespace Engine {
using Microsoft::WRL::ComPtr;

class Model : public GameObject {
  public:
    Model();
    ~Model();

    int GetIndexCount();

    DirectX::SimpleMath::Matrix transform;
    std::vector<std::shared_ptr<Bone>> bones;
    std::vector<std::shared_ptr<Mesh>> meshes;
    //std::shared_ptr<Animation> animation;

    std::shared_ptr<PhongShaderSource> phong_shader_source;
    std::shared_ptr<ImageBasedShaderSource> image_based_shader_source;
    std::shared_ptr<PhsicallyBasedShaderSource> physically_based_shader_source;
};


class ModelDetailNode : public GraphNode {
  public:
    EnumBehaviorTreeStatus OnInvoke() override;
    EnumBehaviorTreeStatus OnShow() override;

    ImVector<LinkInfo> links_;
    int next_link_Id = 100;

    bool use_texture = false;

    bool use_perspective_projection = true;
    DirectX::SimpleMath::Vector3 translation =
        DirectX::SimpleMath::Vector3(0.0f);
    DirectX::SimpleMath::Vector3 rotation = DirectX::SimpleMath::Vector3(0.0f);
    DirectX::SimpleMath::Vector3 scaling = DirectX::SimpleMath::Vector3(1.0f);

    float shininess = 1.0f;
    float diffuse = 1.0f;
    float specular = 1.0f;
};

} // namespace Engine
#endif