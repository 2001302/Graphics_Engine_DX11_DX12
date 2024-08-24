#include "mesh_renderer.h"

namespace core {

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

    dx12::Util::CreateVertexBuffer(meshData.vertices, newMesh->vertexBuffer,
                                   newMesh->vertexBufferView);
    newMesh->indexCount = UINT(meshData.indices.size());
    newMesh->vertexCount = UINT(meshData.vertices.size());
    newMesh->stride = UINT(sizeof(Vertex));
    dx12::Util::CreateIndexBuffer(meshData.indices, newMesh->indexBuffer,
                                  newMesh->indexBufferView);
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

    mesh_consts.GetCpu().world = Matrix();
    mesh_consts.Initialize();
    material_consts.Initialize();

    this->meshes.resize(meshes.size());
    for (int i = 0; i < meshes.size(); i++) {

        MeshData meshData = meshes[i];
        this->meshes[i] = std::make_shared<Mesh>();

        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.NumDescriptors = 5;
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        dx12::ThrowIfFailed(
            dx12::GpuCore::Instance().device->CreateDescriptorHeap(
                &heapDesc, IID_PPV_ARGS(&this->meshes[i]->texture_heap)));
        CD3DX12_CPU_DESCRIPTOR_HANDLE texture_handle(
            this->meshes[i]
                ->texture_heap->GetCPUDescriptorHandleForHeapStart());

        InitMeshBuffers(meshData, this->meshes[i]);

        if (!meshData.albedoTextureFilename.empty()) {
            if (filesystem::exists(meshData.albedoTextureFilename)) {
                if (!meshData.opacityTextureFilename.empty()) {
                    dx12::Util::CreateTexture(
                        meshData.albedoTextureFilename,
                        meshData.opacityTextureFilename, false,
                        this->meshes[i]->albedoTexture, texture_handle);
                } else {
                    dx12::Util::CreateTexture(
                        meshData.albedoTextureFilename, true,
                        this->meshes[i]->albedoTexture, texture_handle);
                }
                material_consts.GetCpu().useAlbedoMap = true;
            } else {
                cout << meshData.albedoTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.emissiveTextureFilename.empty()) {
            if (filesystem::exists(meshData.emissiveTextureFilename)) {
                dx12::Util::CreateTexture(
                    meshData.emissiveTextureFilename, true,
                    this->meshes[i]->emissiveTexture, texture_handle);
                material_consts.GetCpu().useEmissiveMap = true;
            } else {
                cout << meshData.emissiveTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.normalTextureFilename.empty()) {
            if (filesystem::exists(meshData.normalTextureFilename)) {
                dx12::Util::CreateTexture(meshData.normalTextureFilename, false,
                                          this->meshes[i]->normalTexture,
                                          texture_handle);
                material_consts.GetCpu().useNormalMap = true;
            } else {
                cout << meshData.normalTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.heightTextureFilename.empty()) {
            if (filesystem::exists(meshData.heightTextureFilename)) {
                dx12::Util::CreateTexture(meshData.heightTextureFilename, false,
                                          this->meshes[i]->heightTexture,
                                          texture_handle);
                mesh_consts.GetCpu().useHeightMap = true;
            } else {
                cout << meshData.heightTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
        }

        if (!meshData.aoTextureFilename.empty()) {
            if (filesystem::exists(meshData.aoTextureFilename)) {
                dx12::Util::CreateTexture(meshData.aoTextureFilename, false,
                                          this->meshes[i]->aoTexture,
                                          texture_handle);
                material_consts.GetCpu().useAOMap = true;
            } else {
                cout << meshData.aoTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
        }

        // Green : Roughness, Blue : Metallic(Metalness)
        if (!meshData.metallicTextureFilename.empty() ||
            !meshData.roughnessTextureFilename.empty()) {

            // if (filesystem::exists(meshData.metallicTextureFilename) &&
            //     filesystem::exists(meshData.roughnessTextureFilename)) {

            //    dx12::Util::CreateMetallicRoughnessTexture(
            //        meshData.metallicTextureFilename,
            //        meshData.roughnessTextureFilename,
            //        this->meshes[i]->metallicRoughnessTexture,
            //        this->meshes[i]->metallicRoughnessSRV);
            //} else {
            //    cout << meshData.metallicTextureFilename << " or "
            //         << meshData.roughnessTextureFilename
            //         << " does not exists. Skip texture reading." << endl;
            //}
        }

        if (!meshData.metallicTextureFilename.empty()) {
            material_consts.GetCpu().useMetallicMap = true;
        }

        if (!meshData.roughnessTextureFilename.empty()) {
            material_consts.GetCpu().useRoughnessMap = true;
        }

        this->meshes[i]->meshConstsGPU = mesh_consts.Get();
        this->meshes[i]->materialConstsGPU = material_consts.Get();
    }
}

void MeshRenderer::UpdateConstantBuffers() {
    if (is_visible) {
        mesh_consts.Upload();
        material_consts.Upload();
    }
}

void MeshRenderer::Render(ID3D12GraphicsCommandList *command_list) {
    if (is_visible) {
        for (const auto &mesh : meshes) {
            command_list->IASetPrimitiveTopology(
                D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            command_list->IASetVertexBuffers(0, 1, &mesh->vertexBufferView);
            command_list->IASetIndexBuffer(&mesh->indexBufferView);
            command_list->DrawIndexedInstanced(mesh->indexCount, 1, 0, 0, 0);
        }
    }
}

void MeshRenderer::RenderNormals() {
    // for (const auto &mesh : meshes) {
    //     ID3D11Buffer *constBuffers[2] = {mesh->meshConstsGPU.Get(),
    //                                      mesh->materialConstsGPU.Get()};
    //     dx12::GpuCore::Instance().device_context->GSSetConstantBuffers(
    //         1, 2, constBuffers);
    //     dx12::GpuCore::Instance().device_context->IASetVertexBuffers(
    //         0, 1, mesh->vertexBuffer.GetAddressOf(), &mesh->stride,
    //         &mesh->offset);
    //     dx12::GpuCore::Instance().device_context->Draw(mesh->vertexCount, 0);
    // }
}

void MeshRenderer::RenderWireBoundingBox() {
    // ID3D11Buffer *constBuffers[2] = {
    //     bounding_box_mesh->meshConstsGPU.Get(),
    //     bounding_box_mesh->materialConstsGPU.Get()};
    // dx12::GpuCore::Instance().device_context->VSSetConstantBuffers(
    //     1, 2, constBuffers);
    // dx12::GpuCore::Instance().device_context->IASetVertexBuffers(
    //     0, 1, bounding_box_mesh->vertexBuffer.GetAddressOf(),
    //     &bounding_box_mesh->stride, &bounding_box_mesh->offset);
    // dx12::GpuCore::Instance().device_context->IASetIndexBuffer(
    //     bounding_box_mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    // dx12::GpuCore::Instance().device_context->DrawIndexed(
    //     bounding_box_mesh->indexCount, 0, 0);
}

void MeshRenderer::RenderWireBoundingSphere() {
    // ID3D11Buffer *constBuffers[2] = {
    //     bounding_box_mesh->meshConstsGPU.Get(),
    //     bounding_box_mesh->materialConstsGPU.Get()};
    // dx12::GpuCore::Instance().device_context->VSSetConstantBuffers(
    //     1, 2, constBuffers);
    // dx12::GpuCore::Instance().device_context->IASetVertexBuffers(
    //     0, 1, bounding_sphere_mesh->vertexBuffer.GetAddressOf(),
    //     &bounding_sphere_mesh->stride, &bounding_sphere_mesh->offset);
    // dx12::GpuCore::Instance().device_context->IASetIndexBuffer(
    //     bounding_sphere_mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    // dx12::GpuCore::Instance().device_context->DrawIndexed(
    //     bounding_sphere_mesh->indexCount, 0, 0);
}

void MeshRenderer::UpdateWorldRow(const Matrix &worldRow) {
    this->world_row = worldRow;
    this->world_row_IT = worldRow;
    world_row_IT.Translation(Vector3(0.0f));
    world_row_IT = world_row_IT.Invert().Transpose();

    bounding_sphere.Center = this->world_row.Translation();

    mesh_consts.GetCpu().world = worldRow.Transpose();
    mesh_consts.GetCpu().worldIT = world_row_IT.Transpose();
    mesh_consts.GetCpu().worldInv = mesh_consts.GetCpu().world.Invert();
}

} // namespace core
