#include "mesh.h"
#include "texture_2d.h"

namespace graphics {
void Mesh::Initialize(const MeshData &mesh_data,
                      ConstantBuffer<MeshConstants> &mesh_consts,
                      ConstantBuffer<MaterialConstants> &material_consts,
                      GpuHeap *heap,
                      ComPtr<ID3D12GraphicsCommandList> command_list,
                      bool use_texture) {
    buffer_PS = std::make_shared<GpuBufferList>(heap);
    buffer_VS = std::make_shared<GpuBufferList>(heap);

    Util::CreateVertexBuffer(mesh_data.vertices, vertex_buffer,
                             vertex_buffer_view);
    index_count = UINT(mesh_data.indices.size());
    vertex_count = UINT(mesh_data.vertices.size());
    stride = UINT(sizeof(Vertex));
    Util::CreateIndexBuffer(mesh_data.indices, index_buffer, index_buffer_view);

    auto image =
        Image::Read(mesh_data.albedoTextureFilename,
                    mesh_data.opacityTextureFilename, command_list, false);
    if (!image.IsEmpty()) {
        buffer_PS->Add(std::make_shared<Texture2D>(image, command_list));
    }
    else {
        buffer_PS->Add(
            std::make_shared<Texture2D>(256, 256, DXGI_FORMAT_R8G8B8A8_UNORM));
        std::cout << mesh_data.albedoTextureFilename
                  << " does not exists. Skip texture reading." << std::endl;
    }

    image = Image::Read(mesh_data.normalTextureFilename, command_list, false);
    if (!image.IsEmpty()) {
        buffer_PS->Add(std::make_shared<Texture2D>(image, command_list));
    } else {
        buffer_PS->Add(
            std::make_shared<Texture2D>(256, 256, DXGI_FORMAT_R8G8B8A8_UNORM));
        std::cout << mesh_data.normalTextureFilename
                  << " does not exists. Skip texture reading." << std::endl;
    }

    image = Image::Read(mesh_data.aoTextureFilename, command_list, false);
    if (!image.IsEmpty()) {
        buffer_PS->Add(std::make_shared<Texture2D>(image, command_list));
    } else {
        buffer_PS->Add(
            std::make_shared<Texture2D>(256, 256, DXGI_FORMAT_R8G8B8A8_UNORM));
        std::cout << mesh_data.aoTextureFilename
                  << " does not exists. Skip texture reading." << std::endl;
    }

    image = Image::ReadMetallicRoughness(mesh_data.metallicTextureFilename,
                                         mesh_data.roughnessTextureFilename,
                                         command_list, false);
    if (!image.IsEmpty()) {
        buffer_PS->Add(std::make_shared<Texture2D>(image, command_list));
    } else {
        buffer_PS->Add(
            std::make_shared<Texture2D>(256, 256, DXGI_FORMAT_R8G8B8A8_UNORM));
        std::cout << mesh_data.metallicTextureFilename << " or "
                  << mesh_data.roughnessTextureFilename
                  << " does not exists. Skip texture reading." << std::endl;
    }

    image = Image::Read(mesh_data.emissiveTextureFilename, command_list, false);
    if (!image.IsEmpty()) {
        buffer_PS->Add(std::make_shared<Texture2D>(image, command_list));
    } else {
        buffer_PS->Add(
            std::make_shared<Texture2D>(256, 256, DXGI_FORMAT_R8G8B8A8_UNORM));
        std::cout << mesh_data.emissiveTextureFilename
                  << " does not exists. Skip texture reading." << std::endl;
    }

    image = Image::Read(mesh_data.heightTextureFilename, command_list, false);
    if (!image.IsEmpty()) {
        buffer_VS->Add(std::make_shared<Texture2D>(image, command_list));
    } else {
        buffer_VS->Add(
            std::make_shared<Texture2D>(256, 256, DXGI_FORMAT_R8G8B8A8_UNORM));
        std::cout << mesh_data.heightTextureFilename
                  << " does not exists. Skip texture reading." << std::endl;
    }

    buffer_PS->Allocate();
    buffer_VS->Allocate();
}
} // namespace dx12