#include "skinned_mesh_renderer.h"

namespace core {
SkinnedMeshRenderer::SkinnedMeshRenderer(ComPtr<ID3D11Device> &device,
                                         ComPtr<ID3D11DeviceContext> &context,
                                         const vector<MeshData> &meshes) {
    Initialize(device, context, meshes);
}

void SkinnedMeshRenderer::Initialize(ComPtr<ID3D11Device> &device,
                                     ComPtr<ID3D11DeviceContext> &context,
                                     const vector<MeshData> &meshes) {
    MeshRenderer::Initialize(device, context, meshes);
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

void SkinnedMeshRenderer::InitMeshBuffers(ComPtr<ID3D11Device> &device,
                                          const MeshData &meshData,
                                          std::shared_ptr<Mesh> &newMesh) {
    GraphicsUtil::CreateVertexBuffer(meshData.skinnedVertices,
                                     newMesh->vertexBuffer);
    newMesh->indexCount = UINT(meshData.indices.size());
    newMesh->vertexCount = UINT(meshData.skinnedVertices.size());
    newMesh->stride = UINT(sizeof(SkinnedVertex));
    GraphicsUtil::CreateIndexBuffer(meshData.indices, newMesh->indexBuffer);
}
} // namespace core