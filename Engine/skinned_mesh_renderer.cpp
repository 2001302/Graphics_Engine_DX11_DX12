#include "skinned_mesh_renderer.h"

namespace engine {
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

GraphicsPSO &SkinnedMeshRenderer::GetPSO(const bool wired) {
    return wired ? Graphics::skinnedWirePSO : Graphics::skinnedSolidPSO;
}

GraphicsPSO &SkinnedMeshRenderer::GetReflectPSO(const bool wired) {
    return wired ? Graphics::reflectSkinnedWirePSO
                 : Graphics::reflectSkinnedSolidPSO;
}

GraphicsPSO &SkinnedMeshRenderer::GetDepthOnlyPSO() {
    return Graphics::depthOnlySkinnedPSO;
}

void SkinnedMeshRenderer::InitMeshBuffers(ComPtr<ID3D11Device> &device,
                                          const MeshData &meshData,
                                          std::shared_ptr<Mesh> &newMesh) {
    GraphicsUtil::CreateVertexBuffer(device, meshData.skinnedVertices,
                                     newMesh->vertexBuffer);
    newMesh->indexCount = UINT(meshData.indices.size());
    newMesh->vertexCount = UINT(meshData.skinnedVertices.size());
    newMesh->stride = UINT(sizeof(SkinnedVertex));
    GraphicsUtil::CreateIndexBuffer(device, meshData.indices,
                                    newMesh->indexBuffer);
}
} // namespace engine