#ifndef CAMERA
#define CAMERA

#include "behavior_tree_builder.h"
#include "env.h"
#include "graphics_manager.h"
#include "node_ui.h"

namespace engine {
class Camera : public common::INode {
  public:
    Camera();
    Matrix GetView();
    Vector3 GetPosition();
    Vector3 GetLookAt();
    Matrix GetProjection();

    void SetPosition(Vector3 pos);
    void SetLookAt(Vector3 look);

    void Update();

  private:
    Matrix view;
    Vector3 position;

    Vector3 upVector;
    Vector3 lookAtVector;
    float m_yaw; 
    float m_pitch;

    float projection_fov_angle_y;
    float near_z;
    float far_z;
};

} // namespace engine
#endif
