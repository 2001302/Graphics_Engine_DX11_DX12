#ifndef _RENDERER
#define _RENDERER

#include "bone.h"
#include "constant_buffer.h"
#include "geometry_generator.h"
#include "mesh.h"
#include "node.h"
#include "component.h"

namespace engine {
using Microsoft::WRL::ComPtr;

class Renderer : public Component {
  public:
    Renderer() {}
    Renderer(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
          const std::string &basePath, const std::string &filename);
    Renderer(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
          const std::vector<MeshData> &meshes);

    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context,
                    const std::string &basePath, const std::string &filename);

    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context,
                    const std::vector<MeshData> &meshes);

    void UpdateConstantBuffers(ComPtr<ID3D11Device> &device,
                               ComPtr<ID3D11DeviceContext> &context);

    void Render(ComPtr<ID3D11DeviceContext> &context);

    void RenderNormals(ComPtr<ID3D11DeviceContext> &context);

    void UpdateWorldRow(const Matrix &worldRow);


    Matrix m_worldRow = Matrix();   // Model(Object) To World 
    Matrix m_worldITRow = Matrix(); // InverseTranspose

    MeshConstants m_meshConstsCPU;
    MaterialConstants m_materialConstsCPU;

    bool m_drawNormals = false;
    bool m_isVisible = true;
    bool m_castShadow = true;

    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<std::shared_ptr<Bone>> bones;
    // std::shared_ptr<Animation> animation;

  private:
    void OnShow() override;
    ComPtr<ID3D11Buffer> m_meshConstsGPU;
    ComPtr<ID3D11Buffer> m_materialConstsGPU;
};

} // namespace engine
#endif
