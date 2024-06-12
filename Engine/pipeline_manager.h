#ifndef _PIPELINEMANAGER
#define _PIPELINEMANAGER

#include "camera.h"
#include "dataBlock.h"
#include "direct3D.h"
#include "game_object.h"
#include "input.h"
#include "normal_shader.h"
#include "phong_shader.h"

namespace Engine {
/// <summary>
/// Rendering�� �ʿ��� shared object�� ����.
/// </summary>
class PipelineManager : public IDataBlock {
  public:
    std::vector<GameObject *> models;
    std::unique_ptr<PhongShader> phongShader;
    std::unique_ptr<Camera> camera;
};
} // namespace Engine
#endif