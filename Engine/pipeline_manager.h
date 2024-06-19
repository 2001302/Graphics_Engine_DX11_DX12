#ifndef _PIPELINEMANAGER
#define _PIPELINEMANAGER

#include "camera.h"
#include "dataBlock.h"
#include "direct3D.h"
#include "game_object.h"
#include "input.h"
#include "normal_shader.h"
#include "phong_shader.h"
#include "cube_map.h"
#include "graph.h"

namespace Engine {
/// <summary>
/// Rendering에 필요한 shared object의 집합.
/// </summary>
class PipelineManager : public IDataBlock {
  public:
    std::map<int/*id*/, std::shared_ptr<GameObject>> models;
    std::map<int /*id*/, std::shared_ptr<Graph>> behaviors_;
    std::unique_ptr<CubeMap> cubeMap;
    std::unique_ptr<PhongShader> phongShader;
    std::unique_ptr<Camera> camera;
};
} // namespace Engine
#endif