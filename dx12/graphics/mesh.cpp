#include "mesh.h"
#include "texture_2d.h"

namespace graphics {
void Mesh::Initialize(const MeshData &mesh_data,
                      ConstantBuffer<MeshConstants> &mesh_consts,
                      ConstantBuffer<MaterialConstants> &material_consts,
                      DescriptorHeap *heap,
                      ComPtr<ID3D12GraphicsCommandList> command_list,
                      bool use_texture) {

    Util::CreateVertexBuffer(mesh_data.vertices, vertex_buffer,
                             vertex_buffer_view);
    index_count = UINT(mesh_data.indices.size());
    vertex_count = UINT(mesh_data.vertices.size());
    stride = UINT(sizeof(Vertex));
    Util::CreateIndexBuffer(mesh_data.indices, index_buffer, index_buffer_view);

    // buffer_PS = std::make_shared<GpuResourceList>(heap);
    // buffer_VS = std::make_shared<GpuResourceList>(heap);
    
    //auto image = Image::Read(mesh_data.albedo_name, mesh_data.opacity_name,
    //                         command_list, false);
    //if (!image.IsEmpty()) {
    //    buffer_PS->Add(std::make_shared<Texture2D>(image, command_list));
    //} else {
    //    buffer_PS->Add(
    //        std::make_shared<Texture2D>(256, 256, DXGI_FORMAT_R8G8B8A8_UNORM));
    //    std::cout << mesh_data.albedo_name
    //              << " does not exists. Skip texture reading." << std::endl;
    //}

    //image = Image::Read(mesh_data.normal_name, command_list, false);
    //if (!image.IsEmpty()) {
    //    buffer_PS->Add(std::make_shared<Texture2D>(image, command_list));
    //} else {
    //    buffer_PS->Add(
    //        std::make_shared<Texture2D>(256, 256, DXGI_FORMAT_R8G8B8A8_UNORM));
    //    std::cout << mesh_data.normal_name
    //              << " does not exists. Skip texture reading." << std::endl;
    //}

    //image = Image::Read(mesh_data.ambient_occlusion_name, command_list, false);
    //if (!image.IsEmpty()) {
    //    buffer_PS->Add(std::make_shared<Texture2D>(image, command_list));
    //} else {
    //    buffer_PS->Add(
    //        std::make_shared<Texture2D>(256, 256, DXGI_FORMAT_R8G8B8A8_UNORM));
    //    std::cout << mesh_data.ambient_occlusion_name
    //              << " does not exists. Skip texture reading." << std::endl;
    //}

    //image = Image::ReadMetallicRoughness(
    //    mesh_data.metallic_name, mesh_data.roughness_name, command_list, false);
    //if (!image.IsEmpty()) {
    //    buffer_PS->Add(std::make_shared<Texture2D>(image, command_list));
    //} else {
    //    buffer_PS->Add(
    //        std::make_shared<Texture2D>(256, 256, DXGI_FORMAT_R8G8B8A8_UNORM));
    //    std::cout << mesh_data.metallic_name << " or "
    //              << mesh_data.roughness_name
    //              << " does not exists. Skip texture reading." << std::endl;
    //}

    //image = Image::Read(mesh_data.emissive_name, command_list, false);
    //if (!image.IsEmpty()) {
    //    buffer_PS->Add(std::make_shared<Texture2D>(image, command_list));
    //} else {
    //    buffer_PS->Add(
    //        std::make_shared<Texture2D>(256, 256, DXGI_FORMAT_R8G8B8A8_UNORM));
    //    std::cout << mesh_data.emissive_name
    //              << " does not exists. Skip texture reading." << std::endl;
    //}

    //image = Image::Read(mesh_data.height_name, command_list, false);
    //if (!image.IsEmpty()) {
    //    buffer_VS->Add(std::make_shared<Texture2D>(image, command_list));
    //} else {
    //    buffer_VS->Add(
    //        std::make_shared<Texture2D>(256, 256, DXGI_FORMAT_R8G8B8A8_UNORM));
    //    std::cout << mesh_data.height_name
    //              << " does not exists. Skip texture reading." << std::endl;
    //}

    //buffer_PS->Allocate();
    //buffer_VS->Allocate();
}
} // namespace graphics