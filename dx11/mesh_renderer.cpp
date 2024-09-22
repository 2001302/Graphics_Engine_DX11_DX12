#include "mesh_renderer.h"

namespace graphics {

using namespace std;
using namespace DirectX;

MeshRenderer::MeshRenderer(const std::string &basePath,
                           const std::string &filename) {
    Initialize(basePath, filename);
}

MeshRenderer::MeshRenderer(const std::vector<MeshData> &meshes) {
    Initialize(meshes);
}

void MeshRenderer::Initialize() {
    std::cout << "Renderer::Initialize("
                 ") was not implemented."
              << std::endl;
    exit(-1);
}

void MeshRenderer::InitMeshBuffers(const MeshData &meshData,
                                   shared_ptr<Mesh> &newMesh) {

    graphics::Util::CreateVertexBuffer(meshData.vertices,
                                       newMesh->vertexBuffer);
    newMesh->indexCount = UINT(meshData.indices.size());
    newMesh->vertexCount = UINT(meshData.vertices.size());
    newMesh->stride = UINT(sizeof(Vertex));
    graphics::Util::CreateIndexBuffer(meshData.indices, newMesh->indexBuffer);
}

void MeshRenderer::Initialize(const std::string &basePath,
                              const std::string &filename) {
    auto meshes = GeometryGenerator::ReadFromFile(basePath, filename);
    Initialize(meshes);
}

BoundingBox GetBoundingBox(const vector<Vertex> &vertices) {

    if (vertices.size() == 0)
        return BoundingBox();

    Vector3 minCorner = vertices[0].position;
    Vector3 maxCorner = vertices[0].position;

    for (size_t i = 1; i < vertices.size(); i++) {
        minCorner = Vector3::Min(minCorner, vertices[i].position);
        maxCorner = Vector3::Max(maxCorner, vertices[i].position);
    }

    Vector3 center = (minCorner + maxCorner) * 0.5f;
    Vector3 extents = maxCorner - center;

    return BoundingBox(center, extents);
}

void ExtendBoundingBox(const BoundingBox &inBox, BoundingBox &outBox) {

    Vector3 minCorner = Vector3(inBox.Center) - Vector3(inBox.Extents);
    Vector3 maxCorner = Vector3(inBox.Center) - Vector3(inBox.Extents);

    minCorner = Vector3::Min(minCorner,
                             Vector3(outBox.Center) - Vector3(outBox.Extents));
    maxCorner = Vector3::Max(maxCorner,
                             Vector3(outBox.Center) + Vector3(outBox.Extents));

    outBox.Center = (minCorner + maxCorner) * 0.5f;
    outBox.Extents = maxCorner - outBox.Center;
}

void MeshRenderer::Initialize(const vector<MeshData> &meshes) {

    // 일반적으로는 Mesh들이 m_mesh/materialConsts를 각자 소유 가능
    // 여기서는 한 Renderer 안의 여러 Mesh들이 Consts를 모두 공유

    mesh_consts.GetCpu().world = Matrix();
    mesh_consts.Initialize();
    material_consts.Initialize();

    for (const auto &meshData : meshes) {
        auto newMesh = std::make_shared<Mesh>();

        InitMeshBuffers(meshData, newMesh);

        if (!meshData.albedoTextureFilename.empty()) {
            if (filesystem::exists(meshData.albedoTextureFilename)) {
                if (!meshData.opacityTextureFilename.empty()) {
                    graphics::Util::CreateTexture(
                        meshData.albedoTextureFilename,
                        meshData.opacityTextureFilename, false,
                        newMesh->albedoTexture, newMesh->albedoSRV);
                } else {
                    graphics::Util::CreateTexture(
                        meshData.albedoTextureFilename, true,
                        newMesh->albedoTexture, newMesh->albedoSRV);
                }

                material_consts.GetCpu().useAlbedoMap = true;
            } else {
                cout << meshData.albedoTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.emissiveTextureFilename.empty()) {
            if (filesystem::exists(meshData.emissiveTextureFilename)) {
                graphics::Util::CreateTexture(meshData.emissiveTextureFilename,
                                              true, newMesh->emissiveTexture,
                                              newMesh->emissiveSRV);
                material_consts.GetCpu().useEmissiveMap = true;
            } else {
                cout << meshData.emissiveTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.normalTextureFilename.empty()) {
            if (filesystem::exists(meshData.normalTextureFilename)) {
                graphics::Util::CreateTexture(meshData.normalTextureFilename,
                                              false, newMesh->normalTexture,
                                              newMesh->normalSRV);
                material_consts.GetCpu().useNormalMap = true;
            } else {
                cout << meshData.normalTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.heightTextureFilename.empty()) {
            if (filesystem::exists(meshData.heightTextureFilename)) {
                graphics::Util::CreateTexture(meshData.heightTextureFilename,
                                              false, newMesh->heightTexture,
                                              newMesh->heightSRV);
                mesh_consts.GetCpu().useHeightMap = true;
            } else {
                cout << meshData.heightTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.aoTextureFilename.empty()) {
            if (filesystem::exists(meshData.aoTextureFilename)) {
                graphics::Util::CreateTexture(meshData.aoTextureFilename, false,
                                              newMesh->aoTexture,
                                              newMesh->aoSRV);
                material_consts.GetCpu().useAOMap = true;
            } else {
                cout << meshData.aoTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
        }

        // GLTF 방식으로 Metallic과 Roughness를 한 텍스춰에 넣음
        // Green : Roughness, Blue : Metallic(Metalness)
        if (!meshData.metallicTextureFilename.empty() ||
            !meshData.roughnessTextureFilename.empty()) {

            if (filesystem::exists(meshData.metallicTextureFilename) &&
                filesystem::exists(meshData.roughnessTextureFilename)) {

                graphics::Util::CreateMetallicRoughnessTexture(
                    meshData.metallicTextureFilename,
                    meshData.roughnessTextureFilename,
                    newMesh->metallicRoughnessTexture,
                    newMesh->metallicRoughnessSRV);
            } else {
                cout << meshData.metallicTextureFilename << " or "
                     << meshData.roughnessTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.metallicTextureFilename.empty()) {
            material_consts.GetCpu().useMetallicMap = true;
        }

        if (!meshData.roughnessTextureFilename.empty()) {
            material_consts.GetCpu().useRoughnessMap = true;
        }

        newMesh->meshConstsGPU = mesh_consts.Get();
        newMesh->materialConstsGPU = material_consts.Get();

        this->meshes.push_back(newMesh);
    }

    // Initialize Bounding Box
    {
        bounding_box = GetBoundingBox(meshes[0].vertices);
        for (size_t i = 1; i < meshes.size(); i++) {
            auto bb = GetBoundingBox(meshes[0].vertices);
            ExtendBoundingBox(bb, bounding_box);
        }
        auto meshData = GeometryGenerator::MakeWireBox(
            bounding_box.Center,
            Vector3(bounding_box.Extents) + Vector3(1e-3f));
        bounding_box_mesh = std::make_shared<Mesh>();
        graphics::Util::CreateVertexBuffer(meshData.vertices,
                                           bounding_box_mesh->vertexBuffer);
        bounding_box_mesh->indexCount = UINT(meshData.indices.size());
        bounding_box_mesh->vertexCount = UINT(meshData.vertices.size());
        bounding_box_mesh->stride = UINT(sizeof(Vertex));
        graphics::Util::CreateIndexBuffer(meshData.indices,
                                          bounding_box_mesh->indexBuffer);
        bounding_box_mesh->meshConstsGPU = mesh_consts.Get();
        bounding_box_mesh->materialConstsGPU = material_consts.Get();
    }

    // Initialize Bounding Sphere
    {
        float maxRadius = 0.0f;
        for (auto &mesh : meshes) {
            for (auto &v : mesh.vertices) {
                maxRadius = (std::max)(
                    (Vector3(bounding_box.Center) - v.position).Length(),
                    maxRadius);
            }
        }
        maxRadius += 1e-2f; // 살짝 크게 설정
        bounding_sphere = BoundingSphere(bounding_box.Center, maxRadius);
        auto meshData = GeometryGenerator::MakeWireSphere(
            bounding_sphere.Center, bounding_sphere.Radius);
        bounding_sphere_mesh = std::make_shared<Mesh>();
        graphics::Util::CreateVertexBuffer(meshData.vertices,
                                           bounding_sphere_mesh->vertexBuffer);
        bounding_sphere_mesh->indexCount = UINT(meshData.indices.size());
        bounding_sphere_mesh->vertexCount = UINT(meshData.vertices.size());
        bounding_sphere_mesh->stride = UINT(sizeof(Vertex));
        graphics::Util::CreateIndexBuffer(meshData.indices,
                                          bounding_sphere_mesh->indexBuffer);
        bounding_sphere_mesh->meshConstsGPU = mesh_consts.Get();
        bounding_sphere_mesh->materialConstsGPU = material_consts.Get();
    }
}

void MeshRenderer::UpdateConstantBuffers() {
    if (is_visible) {
        mesh_consts.Upload();
        material_consts.Upload();
    }
}

graphics::PipelineState &MeshRenderer::GetPSO(const bool wired) {
    return wired ? graphics::pipeline::defaultWirePSO
                 : graphics::pipeline::defaultSolidPSO;
}

graphics::PipelineState &MeshRenderer::GetDepthOnlyPSO() {
    return graphics::pipeline::depthOnlyPSO;
}

graphics::PipelineState &MeshRenderer::GetReflectPSO(const bool wired) {
    return wired ? graphics::pipeline::reflectWirePSO
                 : graphics::pipeline::reflectSolidPSO;
}

void MeshRenderer::Render() {
    if (is_visible) {
        for (const auto &mesh : meshes) {

            ID3D11Buffer *constBuffers[2] = {mesh->meshConstsGPU.Get(),
                                             mesh->materialConstsGPU.Get()};
            graphics::GpuCore::Instance()
                .device_context->VSSetConstantBuffers(1, 2, constBuffers);

            graphics::GpuCore::Instance()
                .device_context->VSSetShaderResources(
                    0, 1, mesh->heightSRV.GetAddressOf());

            // 물체 렌더링할 때 여러가지 텍스춰 사용 (t0 부터시작)
            vector<ID3D11ShaderResourceView *> resViews = {
                mesh->albedoSRV.Get(), mesh->normalSRV.Get(), mesh->aoSRV.Get(),
                mesh->metallicRoughnessSRV.Get(), mesh->emissiveSRV.Get()};
            graphics::GpuCore::Instance()
                .device_context->PSSetShaderResources(0, // register(t0)
                                                      UINT(resViews.size()),
                                                      resViews.data());
            graphics::GpuCore::Instance()
                .device_context->PSSetConstantBuffers(1, 2, constBuffers);

            // Volume Rendering
            if (mesh->densityTex.GetSRV())
                graphics::GpuCore::Instance()
                    .device_context->PSSetShaderResources(
                        5, 1, mesh->densityTex.GetAddressOfSRV());
            if (mesh->lightingTex.GetSRV())
                graphics::GpuCore::Instance()
                    .device_context->PSSetShaderResources(
                        6, 1, mesh->lightingTex.GetAddressOfSRV());

            graphics::GpuCore::Instance()
                .device_context->IASetVertexBuffers(
                    0, 1, mesh->vertexBuffer.GetAddressOf(), &mesh->stride,
                    &mesh->offset);
            graphics::GpuCore::Instance().device_context->IASetIndexBuffer(
                mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
            graphics::GpuCore::Instance().device_context->DrawIndexed(
                mesh->indexCount, 0, 0);

            // Release resources
            ID3D11ShaderResourceView *nulls[3] = {NULL, NULL, NULL};
            graphics::GpuCore::Instance()
                .device_context->PSSetShaderResources(5, 3, nulls);
        }
    }
}

void MeshRenderer::RenderNormals() {
    for (const auto &mesh : meshes) {
        ID3D11Buffer *constBuffers[2] = {mesh->meshConstsGPU.Get(),
                                         mesh->materialConstsGPU.Get()};
        graphics::GpuCore::Instance().device_context->GSSetConstantBuffers(
            1, 2, constBuffers);
        graphics::GpuCore::Instance().device_context->IASetVertexBuffers(
            0, 1, mesh->vertexBuffer.GetAddressOf(), &mesh->stride,
            &mesh->offset);
        graphics::GpuCore::Instance().device_context->Draw(
            mesh->vertexCount, 0);
    }
}

void MeshRenderer::RenderWireBoundingBox() {
    ID3D11Buffer *constBuffers[2] = {
        bounding_box_mesh->meshConstsGPU.Get(),
        bounding_box_mesh->materialConstsGPU.Get()};
    graphics::GpuCore::Instance().device_context->VSSetConstantBuffers(
        1, 2, constBuffers);
    graphics::GpuCore::Instance().device_context->IASetVertexBuffers(
        0, 1, bounding_box_mesh->vertexBuffer.GetAddressOf(),
        &bounding_box_mesh->stride, &bounding_box_mesh->offset);
    graphics::GpuCore::Instance().device_context->IASetIndexBuffer(
        bounding_box_mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    graphics::GpuCore::Instance().device_context->DrawIndexed(
        bounding_box_mesh->indexCount, 0, 0);
}

void MeshRenderer::RenderWireBoundingSphere() {
    ID3D11Buffer *constBuffers[2] = {
        bounding_box_mesh->meshConstsGPU.Get(),
        bounding_box_mesh->materialConstsGPU.Get()};
    graphics::GpuCore::Instance().device_context->VSSetConstantBuffers(
        1, 2, constBuffers);
    graphics::GpuCore::Instance().device_context->IASetVertexBuffers(
        0, 1, bounding_sphere_mesh->vertexBuffer.GetAddressOf(),
        &bounding_sphere_mesh->stride, &bounding_sphere_mesh->offset);
    graphics::GpuCore::Instance().device_context->IASetIndexBuffer(
        bounding_sphere_mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    graphics::GpuCore::Instance().device_context->DrawIndexed(
        bounding_sphere_mesh->indexCount, 0, 0);
}

void MeshRenderer::UpdateWorldRow(const Matrix &worldRow) {
    this->world_row = worldRow;
    this->world_row_IT = worldRow;
    world_row_IT.Translation(Vector3(0.0f));
    world_row_IT = world_row_IT.Invert().Transpose();

    // 바운딩스피어 위치 업데이트
    // 스케일까지 고려하고 싶다면 x, y, z 스케일 중 가장 큰 값으로 스케일
    // 구(sphere)라서 회전은 고려할 필요 없음
    bounding_sphere.Center = this->world_row.Translation();

    mesh_consts.GetCpu().world = worldRow.Transpose();
    mesh_consts.GetCpu().worldIT = world_row_IT.Transpose();
    mesh_consts.GetCpu().worldInv = mesh_consts.GetCpu().world.Invert();
}

} // namespace core