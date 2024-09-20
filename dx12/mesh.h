#ifndef _MESH
#define _MESH

#include "constant_buffer.h"
#include "gpu_resource.h"
#include "mesh_util.h"
#include "device_manager.h"
#include "vertex.h"

namespace graphics {
struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<SkinnedVertex> skinned_vertices;
    std::vector<uint32_t> indices;
    std::string albedo_name;
    std::string emissive_name;
    std::string normal_name;
    std::string height_name;
    std::string ambient_occlusion_name;
    std::string metallic_name;
    std::string roughness_name;
    std::string opacity_name;
};

struct Mesh {
    Mesh()
        : vertex_buffer(0), index_buffer(0),
          vertex_buffer_view(D3D12_VERTEX_BUFFER_VIEW()),
          index_buffer_view(D3D12_INDEX_BUFFER_VIEW()) {}

    ComPtr<ID3D12Resource> vertex_buffer;
    ComPtr<ID3D12Resource> index_buffer;
    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;
    D3D12_INDEX_BUFFER_VIEW index_buffer_view;
    UINT index_count = 0;
    UINT vertex_count = 0;
    UINT stride = 0;
    UINT offset = 0;

    std::shared_ptr<GpuResourceList> texture_PS; // t0 ~ t4
    std::shared_ptr<GpuResourceList> texture_VS; // t0

    void Initialize(const MeshData &mesh_data,
                    ConstantBuffer<MeshConstants> &mesh_consts,
                    ConstantBuffer<MaterialConstants> &material_consts);
};

} // namespace dx12
#endif
