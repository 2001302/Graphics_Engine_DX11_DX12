#ifndef _GROUND
#define _GROUND

#include "node_ui.h"
#include "mesh.h"
#include "ground_shader.h"

namespace dx11 {
using Microsoft::WRL::ComPtr;

class Ground : public common::INodeUi {
  public:
    Ground();
    ~Ground();

    int GetIndexCount();

    DirectX::SimpleMath::Matrix transform;
    std::shared_ptr<Mesh> mesh;
};
} // namespace dx11
#endif