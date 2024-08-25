#ifndef _RENDERER
#define _RENDERER

#include "../foundation/node.h"
#include "component.h"
#include "constant_buffer.h"
#include "geometry_generator.h"
#include "mesh.h"
#include "render_info.h"
#include <filesystem>
#include <iostream>

namespace core {
using Microsoft::WRL::ComPtr;

class MeshRenderer : public Component {
  public:
    MeshRenderer() {}
    MeshRenderer(const std::string &basePath, const std::string &filename);
    MeshRenderer(const std::vector<MeshData> &meshes);

    virtual void Initialize();
    void Initialize(const std::string &basePath, const std::string &filename);
    void Initialize(const std::vector<MeshData> &meshes);
    virtual void InitMeshBuffers(const MeshData &meshData,
                                 std::shared_ptr<Mesh> &newMesh);

    void UpdateConstantBuffers();
    void UpdateWorldRow(const Matrix &worldRow);

    virtual void Render(RenderCondition *render_condition,
                        dx12::GraphicsPSO *PSO);

  public:
    Matrix world_row = Matrix();    // Model(Object) To World
    Matrix world_row_IT = Matrix(); // InverseTranspose

    bool draw_normals = false;
    bool is_visible = true;
    bool cast_shadow = true;
    bool is_pickable = false;

    std::vector<std::shared_ptr<Mesh>> meshes;

    ConstantBuffer<MeshConstants> mesh_consts;
    ConstantBuffer<MaterialConstants> material_consts;
};

} // namespace core
#endif
