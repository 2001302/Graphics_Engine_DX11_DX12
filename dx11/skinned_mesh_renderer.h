#ifndef _SKINNED_MESH_RENDERER
#define _SKINNED_MESH_RENDERER

#include "constant_buffer.h"
#include "geometry_generator.h"
#include "mesh.h"
#include "mesh_renderer.h"
#include <component.h>
#include <filesystem>
#include <iostream>
#include <node.h>

namespace graphics {
using Microsoft::WRL::ComPtr;

class SkinnedMeshRenderer : public MeshRenderer {
  public:
    SkinnedMeshRenderer(const vector<MeshData> &meshes);
    void Initialize(const vector<MeshData> &meshes);
    graphics::PipelineState &GetPSO(const bool wired) override;
    graphics::PipelineState &GetReflectPSO(const bool wired) override;
    graphics::PipelineState &GetDepthOnlyPSO() override;
    void InitMeshBuffers(const MeshData &meshData,
                         std::shared_ptr<Mesh> &newMesh) override;
};
} // namespace graphics
#endif
