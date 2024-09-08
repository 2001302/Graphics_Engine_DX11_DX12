#ifndef _RENDERER
#define _RENDERER

#include "component.h"
#include "constant_buffer.h"
#include "foundation/node.h"
#include "geometry_generator.h"
#include "mesh.h"
#include "mesh_pso.h"
#include "render_condition.h"
#include "render_target_object.h"
#include <filesystem>
#include <iostream>

namespace graphics {
using Microsoft::WRL::ComPtr;

class MeshRenderer : public Component {
  public:
    MeshRenderer() {}

    virtual void Initialize();
    void Initialize(const std::string &basePath, const std::string &filename,
                    GpuHeap *heap,
                    ComPtr<ID3D12GraphicsCommandList> command_list);
    void Initialize(const std::vector<MeshData> &meshes, GpuHeap *heap,
                    ComPtr<ID3D12GraphicsCommandList> command_list,
                    bool use_texture = true);

    void UpdateConstantBuffers();
    void UpdateWorldRow(const Matrix &worldRow);

    virtual void Render(RenderCondition *render_condition, SolidMeshPSO *PSO);

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