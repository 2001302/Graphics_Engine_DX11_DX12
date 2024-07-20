#pragma warning(disable : 6385)
#pragma warning(disable : 6386)

#include "model.h"

namespace engine {
Model::Model(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
             const std::string &basePath, const std::string &filename) {
    this->Initialize(device, context, basePath, filename);
}

Model::Model(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
             const std::vector<MeshData> &meshes) {
    this->Initialize(device, context, meshes);
}

void Model::Initialize(ComPtr<ID3D11Device> &device,
                       ComPtr<ID3D11DeviceContext> &context,
                       const std::string &basePath,
                       const std::string &filename) {

    auto meshes = GeometryGenerator::ReadFromFile(basePath, filename);

    Initialize(device, context, meshes);
}

void Model::Initialize(ComPtr<ID3D11Device> &device,
                       ComPtr<ID3D11DeviceContext> &context,
                       const std::vector<MeshData> &meshes) {

    //// ConstantBuffer 만들기
    //m_meshConstsCPU.world = Matrix();

    //GraphicsUtil::CreateConstBuffer(device, m_meshConstsCPU, m_meshConstsGPU);
    //GraphicsUtil::CreateConstBuffer(device, m_materialConstsCPU,
    //                              m_materialConstsGPU);

    for (const auto &meshData : meshes) {
        auto newMesh = std::make_shared<Mesh>();
        GraphicsUtil::CreateVertexBuffer(device, meshData.vertices,
                                         newMesh->vertexBuffer);
        newMesh->indexCount = UINT(meshData.indices.size());
        newMesh->vertexCount = UINT(meshData.vertices.size());
        newMesh->stride = UINT(sizeof(Vertex));
        GraphicsUtil::CreateIndexBuffer(device, meshData.indices,
                                        newMesh->indexBuffer);

        //if (!meshData.albedoTextureFilename.empty()) {
        //    GraphicsUtil::CreateTexture(
        //        device, context, meshData.albedoTextureFilename, true,
        //        newMesh->albedoTexture, newMesh->albedoSRV);
        //    m_materialConstsCPU.useAlbedoMap = true;
        //}

        //if (!meshData.emissiveTextureFilename.empty()) {
        //    GraphicsUtil::CreateTexture(
        //        device, context, meshData.emissiveTextureFilename, true,
        //        newMesh->emissiveTexture, newMesh->emissiveSRV);
        //    m_materialConstsCPU.useEmissiveMap = true;
        //}

        //if (!meshData.normalTextureFilename.empty()) {
        //    GraphicsUtil::CreateTexture(
        //        device, context, meshData.normalTextureFilename, false,
        //        newMesh->normalTexture, newMesh->normalSRV);
        //    m_materialConstsCPU.useNormalMap = true;
        //}

        //if (!meshData.heightTextureFilename.empty()) {
        //    GraphicsUtil::CreateTexture(
        //        device, context, meshData.heightTextureFilename, false,
        //        newMesh->heightTexture, newMesh->heightSRV);
        //    m_meshConstsCPU.useHeightMap = true;
        //}

        //if (!meshData.aoTextureFilename.empty()) {
        //    GraphicsUtil::CreateTexture(device, context,
        //                                meshData.aoTextureFilename, false,
        //                                newMesh->aoTexture, newMesh->aoSRV);
        //    m_materialConstsCPU.useAOMap = true;
        //}

        //// GLTF 방식으로 Metallic과 Roughness를 한 텍스춰에 넣음
        //// Green : Roughness, Blue : Metallic(Metalness)
        //if (!meshData.metallicTextureFilename.empty() ||
        //    !meshData.roughnessTextureFilename.empty()) {
        //    GraphicsUtil::CreateMetallicRoughnessTexture(
        //        device, context, meshData.metallicTextureFilename,
        //        meshData.roughnessTextureFilename,
        //        newMesh->metallicRoughnessTexture,
        //        newMesh->metallicRoughnessSRV);
        //}

        //if (!meshData.metallicTextureFilename.empty()) {
        //    m_materialConstsCPU.useMetallicMap = true;
        //}

        //if (!meshData.roughnessTextureFilename.empty()) {
        //    m_materialConstsCPU.useRoughnessMap = true;
        //}

        //newMesh->vertexConstBuffer = m_meshConstsGPU;
        //newMesh->pixelConstBuffer = m_materialConstsGPU;

        this->meshes.push_back(newMesh);
    }
}

void Model::OnShow() {
    ImGui::Text("Transform");
    ImGui::SliderFloat3("Translation", &translation.x, -2.0f, 2.0f);
    ImGui::SliderFloat("Rotation", &rotation.y, -3.14f, 3.14f);
    ImGui::SliderFloat3("Scaling", &scaling.x, 0.1f, 4.0f);

    if (ImGui::RadioButton("Use Phong",
                           render_mode == EnumRenderMode::eLight)) {
        render_mode = EnumRenderMode::eLight;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Use PBR",
                           render_mode ==
                               EnumRenderMode::ePhysicallyBasedRendering)) {
        render_mode = EnumRenderMode::ePhysicallyBasedRendering;
    }
}
} // namespace engine