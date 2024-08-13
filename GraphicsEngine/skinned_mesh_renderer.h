#ifndef _SKINNED_MESH_RENDERER
#define _SKINNED_MESH_RENDERER

#include "component.h"
#include "constant_buffer.h"
#include "geometry_generator.h"
#include "mesh.h"
#include "node.h"
#include "mesh_renderer.h"
#include <filesystem>
#include <iostream>

namespace core {
using Microsoft::WRL::ComPtr;

class SkinnedMeshRenderer : public MeshRenderer {
  public:
    SkinnedMeshRenderer(ComPtr<ID3D11Device> &device,
                        ComPtr<ID3D11DeviceContext> &context,
                        const vector<MeshData> &meshes);
    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context,
                    const vector<MeshData> &meshes);
    PipelineState &GetPSO(const bool wired) override;
    PipelineState &GetReflectPSO(const bool wired) override;
    PipelineState &GetDepthOnlyPSO() override;
    void InitMeshBuffers(ComPtr<ID3D11Device> &device, const MeshData &meshData,
                         std::shared_ptr<Mesh> &newMesh) override;
};
} // namespace engine
#endif
