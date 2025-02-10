#include "skinned_mesh_renderer.h"

namespace graphics {
SkinnedMeshRenderer::SkinnedMeshRenderer(const vector<MeshData> &meshes) {
    Initialize(meshes);
}

void SkinnedMeshRenderer::Initialize(const vector<MeshData> &meshes) {
    MeshRenderer::Initialize(meshes);
}

graphics::PipelineState &SkinnedMeshRenderer::GetPSO(const bool wired) {
    return wired ? graphics::pipeline::skinnedWirePSO
                 : graphics::pipeline::skinnedSolidPSO;
}

graphics::PipelineState &SkinnedMeshRenderer::GetReflectPSO(const bool wired) {
    return wired ? graphics::pipeline::reflectSkinnedWirePSO
                 : graphics::pipeline::reflectSkinnedSolidPSO;
}

graphics::PipelineState &SkinnedMeshRenderer::GetDepthOnlyPSO() {
    return graphics::pipeline::depthOnlySkinnedPSO;
}

void SkinnedMeshRenderer::InitMeshBuffers(const MeshData &meshData,
                                          std::shared_ptr<Mesh> &newMesh) {
    graphics::Util::CreateVertexBuffer(meshData.skinnedVertices,
                                       newMesh->vertexBuffer);
    newMesh->indexCount = UINT(meshData.indices.size());
    newMesh->vertexCount = UINT(meshData.skinnedVertices.size());
    newMesh->stride = UINT(sizeof(SkinnedVertex));
    graphics::Util::CreateIndexBuffer(meshData.indices, newMesh->indexBuffer);
}
} // namespace core