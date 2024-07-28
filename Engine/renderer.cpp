#include "renderer.h"

namespace engine {
Renderer::Renderer(ComPtr<ID3D11Device> &device,
                   ComPtr<ID3D11DeviceContext> &context,
                   const std::string &basePath, const std::string &filename) {
    this->Initialize(device, context, basePath, filename);
}

Renderer::Renderer(ComPtr<ID3D11Device> &device,
                   ComPtr<ID3D11DeviceContext> &context,
                   const std::vector<MeshData> &meshes) {
    this->Initialize(device, context, meshes);
}

void Renderer::Initialize(ComPtr<ID3D11Device> &device,
                          ComPtr<ID3D11DeviceContext> &context,
                          const std::string &basePath,
                          const std::string &filename) {

    auto meshes = GeometryGenerator::ReadFromFile(basePath, filename);

    Initialize(device, context, meshes);
}

void Renderer::Initialize(ComPtr<ID3D11Device> &device,
                          ComPtr<ID3D11DeviceContext> &context,
                          const std::vector<MeshData> &meshes) {

    m_meshConstsCPU.world = Matrix();

    GraphicsUtil::CreateConstBuffer(device, m_meshConstsCPU, m_meshConstsGPU);
    GraphicsUtil::CreateConstBuffer(device, m_materialConstsCPU,
                                    m_materialConstsGPU);

    for (const auto &meshData : meshes) {
        auto newMesh = std::make_shared<Mesh>();
        GraphicsUtil::CreateVertexBuffer(device, meshData.vertices,
                                         newMesh->vertexBuffer);
        newMesh->indexCount = UINT(meshData.indices.size());
        newMesh->vertexCount = UINT(meshData.vertices.size());
        newMesh->stride = UINT(sizeof(Vertex));
        GraphicsUtil::CreateIndexBuffer(device, meshData.indices,
                                        newMesh->indexBuffer);

        if (!meshData.albedoTextureFilename.empty()) {
            GraphicsUtil::CreateTexture(
                device, context, meshData.albedoTextureFilename, true,
                newMesh->albedoTexture, newMesh->albedoSRV);
            m_materialConstsCPU.useAlbedoMap = true;
        }

        if (!meshData.emissiveTextureFilename.empty()) {
            GraphicsUtil::CreateTexture(
                device, context, meshData.emissiveTextureFilename, true,
                newMesh->emissiveTexture, newMesh->emissiveSRV);
            m_materialConstsCPU.useEmissiveMap = true;
        }

        if (!meshData.normalTextureFilename.empty()) {
            GraphicsUtil::CreateTexture(
                device, context, meshData.normalTextureFilename, false,
                newMesh->normalTexture, newMesh->normalSRV);
            m_materialConstsCPU.useNormalMap = true;
        }

        if (!meshData.heightTextureFilename.empty()) {
            GraphicsUtil::CreateTexture(
                device, context, meshData.heightTextureFilename, false,
                newMesh->heightTexture, newMesh->heightSRV);
            m_meshConstsCPU.useHeightMap = true;
        }

        if (!meshData.aoTextureFilename.empty()) {
            GraphicsUtil::CreateTexture(device, context,
                                        meshData.aoTextureFilename, false,
                                        newMesh->aoTexture, newMesh->aoSRV);
            m_materialConstsCPU.useAOMap = true;
        }

        // GLTF : Metallic + Roughness
        // Green : Roughness, Blue : Metallic(Metalness)
        if (!meshData.metallicTextureFilename.empty() ||
            !meshData.roughnessTextureFilename.empty()) {
            GraphicsUtil::CreateMetallicRoughnessTexture(
                device, context, meshData.metallicTextureFilename,
                meshData.roughnessTextureFilename,
                newMesh->metallicRoughnessTexture,
                newMesh->metallicRoughnessSRV);
        }

        if (!meshData.metallicTextureFilename.empty()) {
            m_materialConstsCPU.useMetallicMap = true;
        }

        if (!meshData.roughnessTextureFilename.empty()) {
            m_materialConstsCPU.useRoughnessMap = true;
        }

        newMesh->vertexConstBuffer = m_meshConstsGPU;
        newMesh->pixelConstBuffer = m_materialConstsGPU;

        this->meshes.push_back(newMesh);
    }
}

void Renderer::UpdateConstantBuffers(ComPtr<ID3D11Device> &device,
                                     ComPtr<ID3D11DeviceContext> &context) {
    if (m_isVisible) {
        GraphicsUtil::UpdateBuffer(device, context, m_meshConstsCPU,
                                   m_meshConstsGPU);

        GraphicsUtil::UpdateBuffer(device, context, m_materialConstsCPU,
                                   m_materialConstsGPU);
    }
}

void Renderer::Render(ComPtr<ID3D11DeviceContext> &context) {
    if (m_isVisible) {
        for (const auto &mesh : meshes) {
            context->VSSetConstantBuffers(
                0, 1, mesh->vertexConstBuffer.GetAddressOf());
            context->PSSetConstantBuffers(
                0, 1, mesh->pixelConstBuffer.GetAddressOf());

            context->VSSetShaderResources(0, 1, mesh->heightSRV.GetAddressOf());

            // Start from t0
            std::vector<ID3D11ShaderResourceView *> resViews = {
                mesh->albedoSRV.Get(), mesh->normalSRV.Get(), mesh->aoSRV.Get(),
                mesh->metallicRoughnessSRV.Get(), mesh->emissiveSRV.Get()};
            context->PSSetShaderResources(0, UINT(resViews.size()),
                                          resViews.data());

            context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(),
                                        &mesh->stride, &mesh->offset);

            context->IASetIndexBuffer(mesh->indexBuffer.Get(),
                                      DXGI_FORMAT_R32_UINT, 0);
            context->DrawIndexed(mesh->indexCount, 0, 0);
        }
    }
}

void Renderer::RenderNormals(ComPtr<ID3D11DeviceContext> &context) {
    for (const auto &mesh : meshes) {
        context->GSSetConstantBuffers(0, 1, m_meshConstsGPU.GetAddressOf());
        context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(),
                                    &mesh->stride, &mesh->offset);
        context->Draw(mesh->vertexCount, 0);
    }
}

void Renderer::UpdateWorldRow(const Matrix &worldRow) {
    this->m_worldRow = worldRow;
    this->m_worldITRow = worldRow;
    m_worldITRow.Translation(Vector3(0.0f));
    m_worldITRow = m_worldITRow.Invert().Transpose();

    m_meshConstsCPU.world = worldRow.Transpose();
    m_meshConstsCPU.worldIT = m_worldITRow.Transpose();
}

void Renderer::OnShow() {

}
} // namespace engine