#ifndef _GRAPHICSPSO
#define _GRAPHICSPSO

#include "d3dx12.h"
#include <d3d12.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <directxtk/SimpleMath.h>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using Microsoft::WRL::ComPtr;

namespace graphics {
class PipelineStateObject {
  public:
    PipelineStateObject() : root_signature(nullptr), pipeline_state(nullptr) {}
    virtual void Initialize(){};

    ID3D12RootSignature *root_signature;
    ID3D12PipelineState *pipeline_state;
};

class GraphicsPSO : public PipelineStateObject {
  public:
    D3D12_GRAPHICS_PIPELINE_STATE_DESC description;
};

class ComputePSO : public PipelineStateObject {
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
extern D3D12_RASTERIZER_DESC solidCCWRS;
extern D3D12_RASTERIZER_DESC wireRS;
extern D3D12_RASTERIZER_DESC wireCCWRS;
extern D3D12_RASTERIZER_DESC postRS;
} // namespace rasterizer

namespace depth {
extern D3D12_DEPTH_STENCIL_DESC drawDSS;        // generally drawing
extern D3D12_DEPTH_STENCIL_DESC maskDSS;		// displayed in stencil buffer
extern D3D12_DEPTH_STENCIL_DESC drawMaskedDSS;  // stencil marked only
} // namespace depth

} // namespace graphics
#endif
