#include "skinned_mesh_renderer.h"

namespace core {
SkinnedMeshRenderer::SkinnedMeshRenderer(const vector<MeshData> &meshes) {
    Initialize(meshes);
}

void SkinnedMeshRenderer::Initialize(const vector<MeshData> &meshes) {
    MeshRenderer::Initialize(meshes);
}

PipelineState &SkinnedMeshRenderer::GetPSO(const bool wired) {
    return wired ? graphics::skinnedWirePSO : graphics::skinnedSolidPSO;
}

PipelineState &SkinnedMeshRenderer::GetReflectPSO(const bool wired) {
    return wired ? graphics::reflectSkinnedWirePSO
                 : graphics::reflectSkinnedSolidPSO;
}

PipelineState &SkinnedMeshRenderer::GetDepthOnlyPSO() {
    return graphics::depthOnlySkinnedPSO;
}

void SkinnedMeshRenderer::InitMeshBuffers(const MeshData &meshData,
                                          std::shared_ptr<Mesh> &newMesh) {
    GraphicsUtil::CreateVertexBuffer(meshData.skinnedVertices,
                                     newMesh->vertexBuffer);
    newMesh->indexCount = UINT(meshData.indices.size());
    newMesh->vertexCount = UINT(meshData.skinnedVertices.size());
    newMesh->stride = UINT(sizeof(SkinnedVertex));
    GraphicsUtil::CreateIndexBuffer(meshData.indices, newMesh->indexBuffer);
}
} // namespace core