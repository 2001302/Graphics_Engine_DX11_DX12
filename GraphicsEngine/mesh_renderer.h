#ifndef _RENDERER
#define _RENDERER

#include "component.h"
#include "constant_buffer.h"
#include "geometry_generator.h"
#include "mesh.h"
#include "node.h"
#include <filesystem>
#include <iostream>

namespace core {
using Microsoft::WRL::ComPtr;

class MeshRenderer : public Component {
  public:
    MeshRenderer() {}
    MeshRenderer(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
             const std::string &basePath, const std::string &filename);
    MeshRenderer(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
             const std::vector<MeshData> &meshes);

    virtual void Initialize(ComPtr<ID3D11Device> &device,
                            ComPtr<ID3D11DeviceContext> &context);

    virtual void InitMeshBuffers(ComPtr<ID3D11Device> &device,
                                 const MeshData &meshData,
                                 std::shared_ptr<Mesh> &newMesh);

    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context,
                    const std::string &basePath, const std::string &filename);

    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context,
                    const std::vector<MeshData> &meshes);

    void UpdateConstantBuffers(ComPtr<ID3D11Device> &device,
                               ComPtr<ID3D11DeviceContext> &context);

    virtual PipelineState &GetPSO(const bool wired);
    virtual PipelineState &GetDepthOnlyPSO();
    virtual PipelineState &GetReflectPSO(const bool wired);

    virtual void Render(ComPtr<ID3D11DeviceContext> &context);
    virtual void RenderNormals(ComPtr<ID3D11DeviceContext> &context);
    virtual void RenderWireBoundingBox(ComPtr<ID3D11DeviceContext> &context);
    virtual void RenderWireBoundingSphere(ComPtr<ID3D11DeviceContext> &context);
    void UpdateWorldRow(const Matrix &worldRow);

  public:
    Matrix world_row = Matrix();   // Model(Object) To World
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

} // namespace engine
#endif
