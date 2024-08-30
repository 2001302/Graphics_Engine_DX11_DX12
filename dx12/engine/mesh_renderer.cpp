#include "mesh_renderer.h"

namespace core {

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

void MeshRenderer::Initialize(const vector<MeshData> &mesh_data,
                              ComPtr<ID3D12GraphicsCommandList> command_list) {

    meshes.resize(mesh_data.size());
    for (int i = 0; i < mesh_data.size(); i++) {

        MeshData meshData = mesh_data[i];
        meshes[i] = std::make_shared<Mesh>();
        meshes[i]->Initialize(meshData, command_list);

        material_consts.GetCpu().useAlbedoMap =
            meshes[i]->albedo_texture->is_initialized;
        material_consts.GetCpu().useNormalMap =
            meshes[i]->normal_texture->is_initialized;
        material_consts.GetCpu().useEmissiveMap =
            meshes[i]->emissive_texture->is_initialized;
        material_consts.GetCpu().useMetallicMap =
            meshes[i]->metallic_roughness_texture->is_initialized;
        material_consts.GetCpu().useRoughnessMap =
            meshes[i]->metallic_roughness_texture->is_initialized;
        mesh_consts.GetCpu().useHeightMap =
            meshes[i]->height_texture->is_initialized;
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
            PSO->Render(command_list, dx12::GpuCore::Instance().GetHandleHDR(),
                        dx12::GpuCore::Instance().GetHandleDSV(), mesh->heap_PS,
                        mesh->heap_VS, render_condition->sampler_heap,
                        render_condition->global_consts_GPU, mesh_consts.Get(),
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
