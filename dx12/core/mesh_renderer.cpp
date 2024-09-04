#include "mesh_renderer.h"

namespace core {

void MeshRenderer::Initialize() {
    std::cout << "Renderer::Initialize("
                 ") was not implemented."
              << std::endl;
    exit(-1);
}

void MeshRenderer::Initialize(const std::string &basePath,
                              const std::string &filename, dx12::GpuHeap *heap,
                              ComPtr<ID3D12GraphicsCommandList> command_list) {
    auto meshes = GeometryGenerator::ReadFromFile(basePath, filename);
    Initialize(meshes, heap, command_list);
}

void MeshRenderer::Initialize(const vector<dx12::MeshData> &mesh_data,
                              dx12::GpuHeap *heap,
                              ComPtr<ID3D12GraphicsCommandList> command_list,
                              bool use_texture) {

    meshes.resize(mesh_data.size());
    for (int i = 0; i < mesh_data.size(); i++) {

        dx12::MeshData meshData = mesh_data[i];
        meshes[i] = std::make_shared<dx12::Mesh>();
        meshes[i]->Initialize(meshData, mesh_consts, material_consts, heap,
                              command_list, use_texture);
        mesh_consts.GetCpu().world = Matrix();
        material_consts.Initialize();
        mesh_consts.Initialize();
    }
}

void MeshRenderer::UpdateConstantBuffers() {
    if (is_visible) {
        mesh_consts.Upload();
        material_consts.Upload();
    }
}

void MeshRenderer::Render(RenderCondition *render_condition,
                          dx12::SolidMeshPSO *PSO) {
    if (is_visible) {

        auto command_list = render_condition->command_pool->Get(0);
        for (const auto &mesh : meshes) {
            PSO->Render(
                command_list, dx12::GpuCore::Instance().GetHandleHDR(),
                dx12::GpuCore::Instance().GetHandleDSV(),
                render_condition->shared_texture.get(), mesh->buffer_PS.get(),
                mesh->buffer_VS.get(), render_condition->gpu_heap.get(),
                render_condition->global_consts.Get(), mesh_consts.Get(),
                material_consts.Get(), mesh->vertex_buffer_view,
                mesh->index_buffer_view, mesh->index_count);
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
