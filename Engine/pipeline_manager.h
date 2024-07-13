#ifndef _PIPELINEMANAGER
#define _PIPELINEMANAGER

#include "direct3D.h"
#include "dataBlock.h"
#include "camera.h"
#include "model.h"
#include "cube_map.h"
#include "board_map.h"
#include "ground.h"

namespace dx11 {
/// <summary>
/// Rendering에 필요한 shared object의 집합.
/// </summary>
class PipelineManager : public common::IDataBlock {
  public:
    std::unique_ptr<Camera> camera;
    std::unique_ptr<BoardMap> board_map;
    std::shared_ptr<CubeMap> cube_map;
    std::shared_ptr<Ground> ground;
    std::unordered_map<int /*id*/, std::shared_ptr<common::INodeUi>> models;
    std::unordered_map<EnumShaderType, std::shared_ptr<IShader>> shaders;
};
} // namespace Engine
#endif
