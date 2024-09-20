#include "mesh_renderer.h"

namespace graphics {

void MeshRenderer::Initialize() {
    std::cout << "Renderer::Initialize("
                 ") was not implemented."
              << std::endl;
    exit(-1);
}

void MeshRenderer::Initialize(const std::string &basePath,
                              const std::string &filename) {
    auto meshes = GeometryGenerator::ReadFromFile(basePath, filename);
    Initialize(meshes);
}

void MeshRenderer::Initialize(const vector<MeshData> &mesh_data) {

    meshes.resize(mesh_data.size());
    for (int i = 0; i < mesh_data.size(); i++) {

        MeshData meshData = mesh_data[i];
        meshes[i] = std::make_shared<Mesh>();
        meshes[i]->Initialize(meshData, mesh_consts, material_consts);
        material_consts.Initialize();
        mesh_consts.Initialize();
    }
}

void MeshRenderer::UpdateConstantBuffers() {

    mesh_consts.Upload();
    material_consts.Upload();
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

} // namespace graphics
