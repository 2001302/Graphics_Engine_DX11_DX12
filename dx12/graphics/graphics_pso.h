#ifndef _GRAPHICSPSO
#define _GRAPHICSPSO

#include "d3dx12.h"
#include "graphics_core.h"

/// <summary>
/// shader ���� �ʿ�� �ϴ� ���¸� �����ϴ� ����ü
/// </summary>
namespace dx12 {
class PSO {
  public:
    PSO() : root_signature(nullptr), pipeline_state(nullptr) {}

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

} // namespace dx12
#endif