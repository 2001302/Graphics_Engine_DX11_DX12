#include "mesh.h"
#include "texture_2d.h"

namespace graphics {
void Mesh::Initialize(const MeshData &mesh_data,
                      ConstantBuffer<MeshConstants> &mesh_consts,
                      ConstantBuffer<MaterialConstants> &material_consts,
                      bool use_texture) {

    Util::CreateVertexBuffer(mesh_data.vertices, vertex_buffer,
                             vertex_buffer_view);
    index_count = UINT(mesh_data.indices.size());
    vertex_count = UINT(mesh_data.vertices.size());
    stride = UINT(sizeof(Vertex));
    Util::CreateIndexBuffer(mesh_data.indices, index_buffer, index_buffer_view);

    // texture : util으로 이동?
    auto context =
        GpuCore::Instance().GetCommand()->Begin<GraphicsCommandContext>(
            L"Texture2D");

    auto image = Image::Read(mesh_data.albedo_name, mesh_data.opacity_name,
                             context->GetList(), false);
    Texture2D* albedo= new Texture2D();

    if (!image.IsEmpty()) {
        albedo->Create(GpuCore::Instance().GetDevice(),
                      GpuCore::Instance().GetHeap().View(), image,
                      context->GetList());
    } else {
        albedo->Create(GpuCore::Instance().GetDevice(),
                      GpuCore::Instance().GetHeap().View(), 256, 256,
                      DXGI_FORMAT_R8G8B8A8_UNORM);

        std::cout << mesh_data.albedo_name
                  << " does not exists. Skip texture reading." << std::endl;
    }

    image = Image::Read(mesh_data.normal_name, context->GetList(), false);
    Texture2D* normal= new Texture2D();

    if (!image.IsEmpty()) {
        normal->Create(GpuCore::Instance().GetDevice(),
                      GpuCore::Instance().GetHeap().View(), image,
                      context->GetList());
    } else {
        normal->Create(GpuCore::Instance().GetDevice(),
                      GpuCore::Instance().GetHeap().View(), 256, 256,
                      DXGI_FORMAT_R8G8B8A8_UNORM);
        std::cout << mesh_data.normal_name
                  << " does not exists. Skip texture reading." << std::endl;
    }

    image = Image::Read(mesh_data.ambient_occlusion_name, context->GetList(),
                        false);
    Texture2D* ambient_occlusion= new Texture2D();

    if (!image.IsEmpty()) {
        ambient_occlusion->Create(GpuCore::Instance().GetDevice(),
                                 GpuCore::Instance().GetHeap().View(), image,
                                 context->GetList());
    } else {
        ambient_occlusion->Create(GpuCore::Instance().GetDevice(),
                                 GpuCore::Instance().GetHeap().View(), 256, 256,
                                 DXGI_FORMAT_R8G8B8A8_UNORM);
        std::cout << mesh_data.ambient_occlusion_name
                  << " does not exists. Skip texture reading." << std::endl;
    }

    image = Image::ReadMetallicRoughness(mesh_data.metallic_name,
                                         mesh_data.roughness_name,
                                         context->GetList(), false);
    Texture2D* metallic_roughness= new Texture2D();

    if (!image.IsEmpty()) {
        metallic_roughness->Create(GpuCore::Instance().GetDevice(),
                                  GpuCore::Instance().GetHeap().View(), image,
                                  context->GetList());
    } else {
        metallic_roughness->Create(GpuCore::Instance().GetDevice(),
                                  GpuCore::Instance().GetHeap().View(), 256,
                                  256, DXGI_FORMAT_R8G8B8A8_UNORM);
        std::cout << mesh_data.metallic_name << " or "
                  << mesh_data.roughness_name
                  << " does not exists. Skip texture reading." << std::endl;
    }

    image = Image::Read(mesh_data.emissive_name, context->GetList(), false);
    Texture2D* emissive= new Texture2D();
    if (!image.IsEmpty()) {
        emissive->Create(GpuCore::Instance().GetDevice(),
                        GpuCore::Instance().GetHeap().View(), image,
                        context->GetList());
    } else {
        emissive->Create(GpuCore::Instance().GetDevice(),
                        GpuCore::Instance().GetHeap().View(), 256, 256,
                        DXGI_FORMAT_R8G8B8A8_UNORM);
        std::cout << mesh_data.emissive_name
                  << " does not exists. Skip texture reading." << std::endl;
    }

    image = Image::Read(mesh_data.height_name, context->GetList(), false);
    Texture2D* height= new Texture2D();
    if (!image.IsEmpty()) {
        height->Create(GpuCore::Instance().GetDevice(),
                      GpuCore::Instance().GetHeap().View(), image,
                      context->GetList());
    } else {
        height->Create(GpuCore::Instance().GetDevice(),
                      GpuCore::Instance().GetHeap().View(), 256, 256,
                      DXGI_FORMAT_R8G8B8A8_UNORM);
        std::cout << mesh_data.height_name
                  << " does not exists. Skip texture reading." << std::endl;
    }

    auto tex_PS = std::vector<GpuResource *>{
        std::move(albedo), std::move(normal), std::move(ambient_occlusion),
        std::move(metallic_roughness), std::move(emissive)};
    auto tex_VS = std::vector<GpuResource *>{std::move(height)};

    buffer_PS = std::make_shared<GpuResourceList>(tex_PS);
    buffer_VS = std::make_shared<GpuResourceList>(tex_VS);

    buffer_PS->Allocate();
    buffer_VS->Allocate();

    GpuCore::Instance().GetCommand()->Finish(context, true);
    GpuCore::Instance().GetCommand()->Wait(D3D12_COMMAND_LIST_TYPE_DIRECT);
}
} // namespace graphics