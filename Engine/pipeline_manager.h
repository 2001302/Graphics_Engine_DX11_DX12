#ifndef _PIPELINEMANAGER
#define _PIPELINEMANAGER

#include "camera.h"
#include "cube_map.h"
#include "dataBlock.h"
#include "direct3D.h"
#include "game_object.h"
#include "graph.h"
#include "input.h"
#include "normal_shader.h"
#include "phong_shader.h"
#include "cube_map_shader.h"

namespace Engine {
/// <summary>
/// Rendering에 필요한 shared object의 집합.
/// </summary>
class PipelineManager : public IDataBlock {
  public:
    std::map<int /*id*/, std::shared_ptr<GameObject>> models;
    std::map<int /*id*/, std::shared_ptr<Graph>> behaviors;
    std::shared_ptr<CubeMap> cube_map;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<PhongShader> phong_shader;
    std::unique_ptr<CubeMapShader> cube_map_shader;
};
} // namespace Engine
#endif