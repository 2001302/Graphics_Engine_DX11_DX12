#ifndef _PIPELINEMANAGER
#define _PIPELINEMANAGER

#include "camera.h"
#include "dataBlock.h"
#include "graphics_manager.h"
#include "model.h"
#include "projection.h"

namespace dx11 {
/// <summary>
/// Rendering에 필요한 shared object의 집합.
/// </summary>
class PipelineManager : public common::IDataBlock {
  public:
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Projection> projection;
    //std::unique_ptr<Light> light;
    std::shared_ptr<common::INodeUi> cube_map;
    std::shared_ptr<common::INodeUi> ground;
    std::map<int /*id*/, common::INodeUi*> models;
};
} // namespace dx11
#endif
