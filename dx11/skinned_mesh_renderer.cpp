#include "skinned_mesh_renderer.h"

namespace core {
SkinnedMeshRenderer::SkinnedMeshRenderer(const vector<MeshData> &meshes) {
    Initialize(meshes);
}

void SkinnedMeshRenderer::Initialize(const vector<MeshData> &meshes) {
    MeshRenderer::Initialize(meshes);
}

dx11::PipelineState &SkinnedMeshRenderer::GetPSO(const bool wired) {
    return wired ? dx11::pso::skinnedWirePSO
                 : dx11::pso::skinnedSolidPSO;
}

dx11::PipelineState &SkinnedMeshRenderer::GetReflectPSO(const bool wired) {
    return wired ? dx11::pso::reflectSkinnedWirePSO
                 : dx11::pso::reflectSkinnedSolidPSO;
}

dx11::PipelineState &SkinnedMeshRenderer::GetDepthOnlyPSO() {
    return dx11::pso::depthOnlySkinnedPSO;
}

void SkinnedMeshRenderer::InitMeshBuffers(const MeshData &meshData,
                                          std::shared_ptr<Mesh> &newMesh) {
    dx11::Util::CreateVertexBuffer(meshData.skinnedVertices,
                                       newMesh->vertexBuffer);
    newMesh->indexCount = UINT(meshData.indices.size());
    newMesh->vertexCount = UINT(meshData.skinnedVertices.size());
    newMesh->stride = UINT(sizeof(SkinnedVertex));
    dx11::Util::CreateIndexBuffer(meshData.indices, newMesh->indexBuffer);
}
} // namespace core