#ifndef CAMERA
#define CAMERA

#include "foundation/behavior_tree_builder.h"
#include "foundation/env.h"
#include "graphics/graphics_util.h"
#include "foundation/node.h"
#include "model.h"

namespace core {
class Camera : public foundation::INode {
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
    std::shared_ptr<Model> look_at_target;

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

} // namespace engine
#endif
