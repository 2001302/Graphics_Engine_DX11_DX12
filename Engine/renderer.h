#ifndef _RENDERER
#define _RENDERER

#include "bone.h"
#include "component.h"
#include "constant_buffer.h"
#include "geometry_generator.h"
#include "mesh.h"
#include "node.h"
#include <filesystem>
#include <iostream>

namespace engine {
using Microsoft::WRL::ComPtr;

class Renderer : public Component {
  public:
    Renderer() {}
    Renderer(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
             const std::string &basePath, const std::string &filename);
    Renderer(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
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

    virtual GraphicsPSO &GetPSO(const bool wired);
    virtual GraphicsPSO &GetDepthOnlyPSO();
    virtual GraphicsPSO &GetReflectPSO(const bool wired);

    virtual void Render(ComPtr<ID3D11DeviceContext> &context);
    virtual void UpdateAnimation(ComPtr<ID3D11DeviceContext> &context,
                                 int clipId, int frame);
    virtual void RenderNormals(ComPtr<ID3D11DeviceContext> &context);
    virtual void RenderWireBoundingBox(ComPtr<ID3D11DeviceContext> &context);
    virtual void RenderWireBoundingSphere(ComPtr<ID3D11DeviceContext> &context);
    void UpdateWorldRow(const Matrix &worldRow);

  public:
    Matrix m_worldRow = Matrix();   // Model(Object) To World
    Matrix m_worldITRow = Matrix(); // InverseTranspose

    bool m_drawNormals = false;
    bool m_isVisible = true;
    bool m_castShadow = true;
    bool m_isPickable = false; // 마우스로 선택/조작 가능 여부

    std::vector<std::shared_ptr<Mesh>> m_meshes;

    ConstantBuffer<MeshConstants> m_meshConsts;
    ConstantBuffer<MaterialConstants> m_materialConsts;

    DirectX::BoundingBox m_boundingBox;
    DirectX::BoundingSphere m_boundingSphere;

  private:
    void OnShowNode() override;
    std::shared_ptr<Mesh> m_boundingBoxMesh;
    std::shared_ptr<Mesh> m_boundingSphereMesh;
};

} // namespace engine
#endif
