#ifndef _SKINNED_MESH_RENDERER
#define _SKINNED_MESH_RENDERER

#include "../structure/constant_buffer.h"
#include "../util/geometry_generator.h"
#include "../structure/mesh.h"
#include "mesh_renderer.h"
#include <structure/component.h>
#include <filesystem>
#include <iostream>
#include <structure/node.h>

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
