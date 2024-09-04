#ifndef _RENDERER
#define _RENDERER

#include "component.h"
#include "foundation/node.h"
#include "geometry_generator.h"
#include "graphics/mesh_pso.h"
#include "graphics/constant_buffer.h"
#include "graphics/mesh.h"
#include "render_condition.h"
#include "render_target_object.h"
#include <filesystem>
#include <iostream>

namespace core {
using Microsoft::WRL::ComPtr;

class MeshRenderer : public Component {
  public:
    MeshRenderer() {}

    virtual void Initialize();
    void Initialize(const std::string &basePath, const std::string &filename,
                    dx12::GpuHeap *heap,
                    ComPtr<ID3D12GraphicsCommandList> command_list);
    void Initialize(const std::vector<dx12::MeshData> &meshes,
                    dx12::GpuHeap *heap,
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

    std::vector<std::shared_ptr<dx12::Mesh>> meshes;

    dx12::ConstantBuffer<dx12::MeshConstants> mesh_consts;
    dx12::ConstantBuffer<dx12::MaterialConstants> material_consts;
};

} // namespace core
#endif
