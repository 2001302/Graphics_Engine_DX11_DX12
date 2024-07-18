#ifndef _PIPELINEMANAGER
#define _PIPELINEMANAGER

#include "board_map.h"
#include "camera.h"
#include "dataBlock.h"
#include "direct3D.h"
#include "model.h"

namespace dx11 {
/// <summary>
/// Rendering에 필요한 shared object의 집합.
/// </summary>
class PipelineManager : public common::IDataBlock {
  public:
    std::unique_ptr<Camera> camera;
    //std::unique_ptr<Light> light;
    //std::unique_ptr<Projection> projection;
    //std::unique_ptr<BoardMap> board_map;
    std::shared_ptr<common::INodeUi> cube_map;
    std::shared_ptr<common::INodeUi> ground;
    std::map<int /*id*/, common::INodeUi*> models;
    //std::unordered_map<EnumShaderType, std::shared_ptr<IShader>> shaders;
};
} // namespace dx11
#endif
