#include "mesh_renderer.h"

namespace core {

MeshRenderer::MeshRenderer(const std::string &basePath,
                           const std::string &filename,
                           ComPtr<ID3D12GraphicsCommandList> command_list) {
    Initialize(basePath, filename, command_list);
}

MeshRenderer::MeshRenderer(const std::vector<MeshData> &meshes,
                           ComPtr<ID3D12GraphicsCommandList> command_list) {
    Initialize(meshes, command_list);
}

void MeshRenderer::Initialize() {
    std::cout << "Renderer::Initialize("
                 ") was not implemented."
              << std::endl;
    exit(-1);
}

void MeshRenderer::Initialize(const std::string &basePath,
                              const std::string &filename,
                              ComPtr<ID3D12GraphicsCommandList> command_list) {
    auto meshes = GeometryGenerator::ReadFromFile(basePath, filename);
    Initialize(meshes, command_list);
}

void MeshRenderer::Initialize(const vector<MeshData> &meshes,
                              ComPtr<ID3D12GraphicsCommandList> command_list) {

    this->meshes.resize(meshes.size());
    for (int i = 0; i < meshes.size(); i++) {

        MeshData meshData = meshes[i];
        this->meshes[i] = std::make_shared<Mesh>();
        this->meshes[i]->Initialize(meshData, command_list);

        material_consts.GetCpu().useAlbedoMap =
            this->meshes[i]->albedo_texture->is_initialized;
        
        material_consts.GetCpu().useNormalMap =
            this->meshes[i]->normal_texture->is_initialized;

        material_consts.GetCpu().useEmissiveMap =
            this->meshes[i]->emissive_texture->is_initialized;

        material_consts.GetCpu().useMetallicMap =
            this->meshes[i]->metallic_roughness_texture->is_initialized;

        material_consts.GetCpu().useRoughnessMap =
            this->meshes[i]->metallic_roughness_texture->is_initialized;

        mesh_consts.GetCpu().useHeightMap =
            this->meshes[i]->height_texture->is_initialized;

        mesh_consts.GetCpu().world = Matrix();
        mesh_consts.Initialize();
        material_consts.Initialize();
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
            dx12::GpuCore::Instance().GetHandleHDR();
        CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle =
            dx12::GpuCore::Instance().GetHandleDSV();

        for (const auto &mesh : meshes) {

            auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                dx12::GpuCore::Instance().resource_HDR.Get(),
                D3D12_RESOURCE_STATE_COMMON,
                D3D12_RESOURCE_STATE_RENDER_TARGET);
            command_list->ResourceBarrier(1, &barrier);

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
            // global texture
            command_list->SetGraphicsRootConstantBufferView(
                2, render_condition->global_consts_GPU->GetGPUVirtualAddress());
            command_list->SetGraphicsRootConstantBufferView(
                3, mesh_consts.Get()->GetGPUVirtualAddress());
            command_list->SetGraphicsRootConstantBufferView(
                4, material_consts.Get()->GetGPUVirtualAddress());
            // command_list->SetGraphicsRootDescriptorTable(
            //     5, mesh->heap_VS->GetGPUDescriptorHandleForHeapStart());
            command_list->SetGraphicsRootDescriptorTable(
                6, mesh->heap_PS->GetGPUDescriptorHandleForHeapStart());

            command_list->IASetPrimitiveTopology(
                D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            command_list->IASetVertexBuffers(0, 1, &mesh->vertex_buffer_view);
            command_list->IASetIndexBuffer(&mesh->index_buffer_view);
            command_list->DrawIndexedInstanced(mesh->index_count, 1, 0, 0, 0);

            barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                dx12::GpuCore::Instance().resource_HDR.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                D3D12_RESOURCE_STATE_COMMON);
            command_list->ResourceBarrier(1, &barrier);
        }
    }
}

void MeshRenderer::UpdateWorldRow(const Matrix &worldRow) {
    this->world_row = worldRow;
    this->world_row_IT = worldRow;
    world_row_IT.Translation(Vector3(0.0f));
    world_row_IT = world_row_IT.Invert().Transpose();

    mesh_consts.GetCpu().world = worldRow.Transpose();
    mesh_consts.GetCpu().worldIT = world_row_IT.Transpose();
    mesh_consts.GetCpu().worldInv = mesh_consts.GetCpu().world.Invert();
}

} // namespace core
