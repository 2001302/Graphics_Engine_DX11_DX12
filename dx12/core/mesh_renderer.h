#ifndef _RENDERER
#define _RENDERER

#include "foundation/node.h"
#include "graphics/mesh_pso.h"
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

    virtual void Initialize();
    void Initialize(const std::string &basePath, const std::string &filename,
                    ComPtr<ID3D12GraphicsCommandList> command_list);
    void Initialize(const std::vector<MeshData> &meshes,
                    ComPtr<ID3D12GraphicsCommandList> command_list,
                    bool use_texture = true);

    void UpdateConstantBuffers();
    void UpdateWorldRow(const Matrix &worldRow);

    virtual void Render(RenderCondition *render_condition,
                        dx12::SolidMeshPSO *PSO);

  public:
    Matrix world_row = Matrix();    // Model(Object) To World
    Matrix world_row_IT = Matrix(); // InverseTranspose

    bool is_visible = true;

    std::vector<std::shared_ptr<Mesh>> meshes;

    ConstantBuffer<MeshConstants> mesh_consts;
    ConstantBuffer<MaterialConstants> material_consts;
};

} // namespace core
#endif
