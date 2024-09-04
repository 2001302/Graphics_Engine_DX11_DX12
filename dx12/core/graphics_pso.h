#ifndef _GRAPHICSPSO
#define _GRAPHICSPSO

#include "d3dx12.h"
#include "graphics_core.h"

namespace dx12 {
class PSO {
  public:
    PSO() : root_signature(nullptr), pipeline_state(nullptr) {}
    virtual void Initialize(){};

    ID3D12RootSignature *root_signature;
    ID3D12PipelineState *pipeline_state;
};

class GraphicsPSO : public PSO {
  public:
    D3D12_GRAPHICS_PIPELINE_STATE_DESC description;
};

class ComputePSO : public PSO {
  public:
    D3D12_COMPUTE_PIPELINE_STATE_DESC description;
};

namespace sampler {
extern D3D12_SAMPLER_DESC linearWrapSS;
extern D3D12_SAMPLER_DESC linearClampSS;
extern D3D12_SAMPLER_DESC shadowPointSS;
extern D3D12_SAMPLER_DESC shadowCompareSS;
extern D3D12_SAMPLER_DESC pointWrapSS;
extern D3D12_SAMPLER_DESC linearMirrorSS;
extern D3D12_SAMPLER_DESC pointClampSS;
} // namespace sampler

namespace layout {
extern D3D12_INPUT_ELEMENT_DESC combineIEs[2];
extern D3D12_INPUT_ELEMENT_DESC basicIEs[4];
} // namespace layout

namespace rasterizer {
extern D3D12_RASTERIZER_DESC solidRS;
extern D3D12_RASTERIZER_DESC postRS;
} // namespace rasterizer

namespace depth {
extern D3D12_DEPTH_STENCIL_DESC basicDS;
} // namespace depthstencil

} // namespace dx12
#endif
