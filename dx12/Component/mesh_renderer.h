#ifndef _RENDERER
#define _RENDERER

#include <structure/component.h>
#include "../gpu/buffer/constant_buffer.h"
#include "../util/geometry_generator.h"
#include "../structure/mesh.h"
#include <filesystem>
#include <iostream>
#include <structure/node.h>

namespace graphics {
using Microsoft::WRL::ComPtr;

class MeshRenderer : public common::Component {
  public:
    MeshRenderer() {}

    virtual void Initialize();
    void Initialize(const std::string &basePath, const std::string &filename);
    void Initialize(const std::vector<MeshData> &meshes, bool is_skinned = false);

    void UpdateConstantBuffers();
    void UpdateWorldRow(const Matrix &worldRow);

    std::vector<std::shared_ptr<Mesh>> GetMeshes() { return meshes; }
    Matrix GetWorldRow() { return world_row; }
    ConstantBuffer<MeshConstants> &MeshConsts() { return mesh_consts; }
    ConstantBuffer<MaterialConstants> &MaterialConsts() {
        return material_consts;
    }

  private:
    Matrix world_row = Matrix();    // Model(Object) To World
    Matrix world_row_IT = Matrix(); // InverseTranspose

    std::vector<std::shared_ptr<Mesh>> meshes;

    ConstantBuffer<MeshConstants> mesh_consts;
    ConstantBuffer<MaterialConstants> material_consts;
};

} // namespace graphics
#endif
