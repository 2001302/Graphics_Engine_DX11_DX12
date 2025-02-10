#ifndef _RENDERER
#define _RENDERER

#include "constant_buffer.h"
#include "geometry_generator.h"
#include "mesh.h"
#include <structure/component.h>
#include <filesystem>
#include <iostream>
#include <structure/node.h>

namespace graphics {
using Microsoft::WRL::ComPtr;

class MeshRenderer : public common::Component {
  public:
    MeshRenderer() {}
    MeshRenderer(const std::string &basePath, const std::string &filename);
    MeshRenderer(const std::vector<MeshData> &meshes);

    virtual void Initialize();

    virtual void InitMeshBuffers(const MeshData &meshData,
                                 std::shared_ptr<Mesh> &newMesh);

    void Initialize(const std::string &basePath, const std::string &filename);

    void Initialize(const std::vector<MeshData> &meshes);

    void UpdateConstantBuffers();

    virtual graphics::PipelineState &GetPSO(const bool wired);
    virtual graphics::PipelineState &GetDepthOnlyPSO();
    virtual graphics::PipelineState &GetReflectPSO(const bool wired);

    virtual void Render();
    virtual void RenderNormals();
    virtual void RenderWireBoundingBox();
    virtual void RenderWireBoundingSphere();
    void UpdateWorldRow(const Matrix &worldRow);

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

    DirectX::BoundingBox bounding_box;
    DirectX::BoundingSphere bounding_sphere;

  private:
    std::shared_ptr<Mesh> bounding_box_mesh;
    std::shared_ptr<Mesh> bounding_sphere_mesh;
};

} // namespace graphics
#endif
