#include "mesh.h"
#include "texture_2d.h"

namespace graphics {
void Mesh::Initialize(const MeshData &mesh_data,
                      ConstantBuffer<MeshConstants> &mesh_consts,
                      ConstantBuffer<MaterialConstants> &material_consts) {

    Util::CreateVertexBuffer(mesh_data.vertices, vertex_buffer,
                             vertex_buffer_view);
    index_count = UINT(mesh_data.indices.size());
    vertex_count = UINT(mesh_data.vertices.size());
    stride = UINT(sizeof(Vertex));
    Util::CreateIndexBuffer(mesh_data.indices, index_buffer, index_buffer_view);

    auto albedo =
        Texture2D::Create(mesh_data.albedo_name, mesh_data.opacity_name);
    auto normal = Texture2D::Create(mesh_data.normal_name);
    auto ambient_occlusion =
        Texture2D::Create(mesh_data.ambient_occlusion_name);
    auto metallic_roughness = Texture2D::Create(mesh_data.metallic_name,
                                                mesh_data.roughness_name, true);
    auto emissive = Texture2D::Create(mesh_data.emissive_name);
    auto height = Texture2D::Create(mesh_data.height_name);

    auto tex_PS = std::vector<GpuResource *>{
        std::move(albedo), std::move(normal), std::move(ambient_occlusion),
        std::move(metallic_roughness), std::move(emissive)};
    auto tex_VS = std::vector<GpuResource *>{std::move(height)};

    texture_PS = std::make_shared<GpuResourceList>(tex_PS);
    texture_VS = std::make_shared<GpuResourceList>(tex_VS);
}
} // namespace graphics