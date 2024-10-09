#ifndef _RENDERER
#define _RENDERER

#include <component.h>
#include "constant_buffer.h"
#include "geometry_generator.h"
#include "mesh.h"
#include <filesystem>
#include <iostream>
#include <node.h>

namespace graphics {
using Microsoft::WRL::ComPtr;

class MeshRenderer : public common::Component {
  public:
    MeshRenderer() {}

    virtual void Initialize();
    void Initialize(const std::string &basePath, const std::string &filename);
    void Initialize(const std::vector<MeshData> &meshes);

    void UpdateConstantBuffers();
    void UpdateWorldRow(const Matrix &worldRow);

    std::vector<std::shared_ptr<Mesh>> GetMeshes() { return meshes; }
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
