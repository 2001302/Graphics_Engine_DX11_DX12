#ifndef _PIPELINEMANAGER
#define _PIPELINEMANAGER

#include "camera.h"
#include "cube_map.h"
#include "dataBlock.h"
#include "direct3D.h"
#include "model.h"
#include "graph.h"
#include "input.h"
#include "phong_shader.h"
#include "cube_map_shader.h"
#include "image_based_shader.h"
#include "physically_based_shader.h"
#include "normal_geometry_shader.h"

namespace Engine {
/// <summary>
/// Rendering에 필요한 shared object의 집합.
/// </summary>
class PipelineManager : public IDataBlock {
  public:
    std::unique_ptr<Camera> camera;
    std::shared_ptr<CubeMap> cube_map;
    std::map<int /*id*/, std::shared_ptr<Model>> models;
    std::map<EnumShaderType, std::shared_ptr<IShader>> shaders;
};
} // namespace Engine
#endif
