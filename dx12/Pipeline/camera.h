#ifndef CAMERA
#define CAMERA

#include "../util/mesh_util.h"
#include <behaviortree/behavior_tree_builder.h>
#include <structure/env.h>
#include <structure/model.h>
#include <structure/node.h>

namespace graphics {
class Camera : public common::INode {
  public:
    Camera();
    Matrix GetView();
    Vector3 GetPosition();
    Vector3 GetLookAt();
    Matrix GetProjection();

    void SetPosition(Vector3 pos);
    void SetLookAt(Vector3 look);

    void Initialize();
    void Update();
    void Draw();

  private:
    std::shared_ptr<common::Model> look_at_target;

    Matrix view;
    Vector3 position;

    Vector3 upVector;
    Vector3 lookAtVector;
    float yaw;
    float pitch;

    float projection_fov_angle_y;
    float near_z;
    float far_z;
};

} // namespace graphics
#endif
