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

    dx12::Util::CreateVertexBuffer(meshData.vertices, newMesh->vertex_buffer,
                                   newMesh->vertex_buffer_view);
    newMesh->index_count = UINT(meshData.indices.size());
    newMesh->vertex_count = UINT(meshData.vertices.size());
    newMesh->stride = UINT(sizeof(Vertex));
    dx12::Util::CreateIndexBuffer(meshData.indices, newMesh->index_buffer,
                                  newMesh->index_buffer_view);
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

        InitMeshBuffers(meshData, this->meshes[i]);

        UINT descriptorSize =
            dx12::GpuCore::Instance().device->GetDescriptorHandleIncrementSize(
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping =
            D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        // srvDesc.Format = this->meshes[i]->albedo_texture->GetDesc().Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        // heapPS
        CD3DX12_CPU_DESCRIPTOR_HANDLE texture_handle_PS(
            this->meshes[i]->heap_PS->GetCPUDescriptorHandleForHeapStart());

        if (!meshData.albedoTextureFilename.empty()) {
            if (filesystem::exists(meshData.albedoTextureFilename)) {
                if (!meshData.opacityTextureFilename.empty()) {
                    dx12::Util::CreateTexture(meshData.albedoTextureFilename,
                                              meshData.opacityTextureFilename,
                                              false,
                                              this->meshes[i]->albedo_texture);
                } else {
                    dx12::Util::CreateTexture(meshData.albedoTextureFilename,
                                              true,
                                              this->meshes[i]->albedo_texture);
                }
                material_consts.GetCpu().useAlbedoMap = true;
            } else {
                cout << meshData.albedoTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
            srvDesc.Format = this->meshes[i]->albedo_texture->GetDesc().Format;

            dx12::GpuCore::Instance().device->CreateShaderResourceView(
                this->meshes[i]->albedo_texture.Get(), &srvDesc,
                texture_handle_PS);
            texture_handle_PS.Offset(descriptorSize);
        } else {
            dx12::GpuCore::Instance().device->CreateShaderResourceView(
                nullptr, &srvDesc, texture_handle_PS);
            texture_handle_PS.Offset(descriptorSize);
        }

        if (!meshData.normalTextureFilename.empty()) {
            if (filesystem::exists(meshData.normalTextureFilename)) {
                dx12::Util::CreateTexture(meshData.normalTextureFilename, false,
                                          this->meshes[i]->normal_texture);
                material_consts.GetCpu().useNormalMap = true;
            } else {
                cout << meshData.normalTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
            srvDesc.Format = this->meshes[i]->normal_texture->GetDesc().Format;

            dx12::GpuCore::Instance().device->CreateShaderResourceView(
                this->meshes[i]->normal_texture.Get(), &srvDesc,
                texture_handle_PS);
            texture_handle_PS.Offset(descriptorSize);
        } else {
            dx12::GpuCore::Instance().device->CreateShaderResourceView(
                nullptr, &srvDesc, texture_handle_PS);
            texture_handle_PS.Offset(descriptorSize);
        }

        if (!meshData.aoTextureFilename.empty()) {
            if (filesystem::exists(meshData.aoTextureFilename)) {
                dx12::Util::CreateTexture(meshData.aoTextureFilename, false,
                                          this->meshes[i]->ao_texture);
                material_consts.GetCpu().useAOMap = true;
            } else {
                cout << meshData.aoTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
            srvDesc.Format = this->meshes[i]->ao_texture->GetDesc().Format;

            dx12::GpuCore::Instance().device->CreateShaderResourceView(
                this->meshes[i]->ao_texture.Get(), &srvDesc, texture_handle_PS);
            texture_handle_PS.Offset(descriptorSize);
        } else {
            dx12::GpuCore::Instance().device->CreateShaderResourceView(
                nullptr, &srvDesc, texture_handle_PS);
            texture_handle_PS.Offset(descriptorSize);
        }

        // Green : Roughness, Blue : Metallic(Metalness)
        if (!meshData.metallicTextureFilename.empty() ||
            !meshData.roughnessTextureFilename.empty()) {

            if (filesystem::exists(meshData.metallicTextureFilename) &&
                filesystem::exists(meshData.roughnessTextureFilename)) {

                dx12::Util::CreateMetallicRoughnessTexture(
                    meshData.metallicTextureFilename,
                    meshData.roughnessTextureFilename,
                    this->meshes[i]->metallic_roughness_texture);
            } else {
                cout << meshData.metallicTextureFilename << " or "
                     << meshData.roughnessTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
            srvDesc.Format =
                this->meshes[i]->metallic_roughness_texture->GetDesc().Format;

            dx12::GpuCore::Instance().device->CreateShaderResourceView(
                this->meshes[i]->metallic_roughness_texture.Get(), &srvDesc,
                texture_handle_PS);
            texture_handle_PS.Offset(descriptorSize);
        } else {
            dx12::GpuCore::Instance().device->CreateShaderResourceView(
                nullptr, &srvDesc, texture_handle_PS);
            texture_handle_PS.Offset(descriptorSize);
        }

        if (!meshData.emissiveTextureFilename.empty()) {
            if (filesystem::exists(meshData.emissiveTextureFilename)) {
                dx12::Util::CreateTexture(meshData.emissiveTextureFilename,
                                          true,
                                          this->meshes[i]->emissive_texture);
                material_consts.GetCpu().useEmissiveMap = true;
            } else {
                cout << meshData.emissiveTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
            srvDesc.Format =
                this->meshes[i]->emissive_texture->GetDesc().Format;

            dx12::GpuCore::Instance().device->CreateShaderResourceView(
                this->meshes[i]->emissive_texture.Get(), &srvDesc,
                texture_handle_PS);
            texture_handle_PS.Offset(descriptorSize);
        } else {
            dx12::GpuCore::Instance().device->CreateShaderResourceView(
                nullptr, &srvDesc, texture_handle_PS);
            texture_handle_PS.Offset(descriptorSize);
        }

        if (!meshData.metallicTextureFilename.empty()) {
            material_consts.GetCpu().useMetallicMap = true;
        }

        if (!meshData.roughnessTextureFilename.empty()) {
            material_consts.GetCpu().useRoughnessMap = true;
        }

        // heapVS
        CD3DX12_CPU_DESCRIPTOR_HANDLE texture_handle_VS(
            this->meshes[i]->heap_VS->GetCPUDescriptorHandleForHeapStart());

        if (!meshData.heightTextureFilename.empty()) {
            if (filesystem::exists(meshData.heightTextureFilename)) {
                dx12::Util::CreateTexture(meshData.heightTextureFilename, false,
                                          this->meshes[i]->height_texture);
                mesh_consts.GetCpu().useHeightMap = true;
            } else {
                cout << meshData.heightTextureFilename
                     << " does not exists. Skip texture reading." << endl;
            }
            srvDesc.Format = this->meshes[i]->height_texture->GetDesc().Format;

            dx12::GpuCore::Instance().device->CreateShaderResourceView(
                this->meshes[i]->height_texture.Get(), &srvDesc,
                texture_handle_VS);
            texture_handle_VS.Offset(descriptorSize);
        } else {
            dx12::GpuCore::Instance().device->CreateShaderResourceView(
                nullptr, &srvDesc, texture_handle_VS);
            texture_handle_VS.Offset(descriptorSize);
        }

        this->meshes[i]->mesh_consts_GPU = mesh_consts.Get();
        this->meshes[i]->material_consts_GPU = material_consts.Get();
    }
}

void MeshRenderer::UpdateConstantBuffers() {
    if (is_visible) {
        mesh_consts.Upload();
        material_consts.Upload();
    }
}

void MeshRenderer::Render(RenderCondition *render_condition,
                          dx12::GraphicsPSO *PSO) {

    if (is_visible) {

        auto command_list = render_condition->command_pool->Get(0);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle =
            dx12::GpuCore::Instance().GetHandleRTV();
        CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle =
            dx12::GpuCore::Instance().GetHandleDSV();

        for (const auto &mesh : meshes) {

            ID3D12DescriptorHeap *descriptor_heap[] = {
                render_condition->sampler_heap.Get(), mesh->heap_PS.Get()
                /*,mesh->heap_PS.Get()*/};

            command_list->SetName(L"MeshRenderer::Render");
            command_list->RSSetViewports(1,
                                         &dx12::GpuCore::Instance().viewport);
            command_list->RSSetScissorRects(
                1, &dx12::GpuCore::Instance().scissorRect);
            command_list->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);
            command_list->SetGraphicsRootSignature(PSO->root_signature);
            command_list->SetPipelineState(PSO->pipeline_state);
            command_list->SetDescriptorHeaps(_countof(descriptor_heap),
                                             descriptor_heap);

            command_list->SetGraphicsRootDescriptorTable(
                0, render_condition->sampler_heap
                       ->GetGPUDescriptorHandleForHeapStart());
            //global texture
            command_list->SetGraphicsRootConstantBufferView(
                2, render_condition->global_consts_GPU->GetGPUVirtualAddress());
            command_list->SetGraphicsRootConstantBufferView(
                3, mesh_consts.Get()->GetGPUVirtualAddress());
            command_list->SetGraphicsRootConstantBufferView(
                4, material_consts.Get()->GetGPUVirtualAddress());
            //command_list->SetGraphicsRootDescriptorTable(
            //    5, mesh->heap_VS->GetGPUDescriptorHandleForHeapStart());
            command_list->SetGraphicsRootDescriptorTable(
                6, mesh->heap_PS->GetGPUDescriptorHandleForHeapStart());

            command_list->IASetPrimitiveTopology(
                D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            command_list->IASetVertexBuffers(0, 1, &mesh->vertex_buffer_view);
            command_list->IASetIndexBuffer(&mesh->index_buffer_view);
            command_list->DrawIndexedInstanced(mesh->index_count, 1, 0, 0, 0);
        }
    }
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
