#ifndef _SKYBOX_NODE
#define _SKYBOX_NODE

#include "../foundation/behavior_tree_builder.h"
#include "../graphics/command_pool.h"
#include "../graphics/skybox_pso.h"
#include "black_board.h"
#include "mesh_renderer.h"

namespace core {

class SkyBoxNodeInvoker : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto condition = black_board->conditions.get();
        auto targets = black_board->targets.get();

        auto command_list = condition->command_pool->Get(0);
        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {

            skyboxPSO = std::make_shared<dx12::SkyboxPSO>();
            skyboxPSO->Initialize();

            auto mesh_data = GeometryGenerator::MakeBox(40.0f);
            std::reverse(mesh_data.indices.begin(), mesh_data.indices.end());

            auto renderer = std::make_shared<MeshRenderer>();
            renderer->Initialize(std::vector{mesh_data}, command_list, false);

            skybox = std::make_shared<Model>();
            skybox->AddComponent(EnumComponentType::eRenderer, renderer);

            auto env_name = L"./Assets/Textures/Cubemaps/HDRI/SampleEnvHDR.dds";
            auto specular_name =
                L"./Assets/Textures/Cubemaps/HDRI/SampleSpecularHDR.dds";
            auto irradiance_name =
                L"./Assets/Textures/Cubemaps/HDRI/SampleDiffuseHDR.dds";
            auto brdf_name = L"./Assets/Textures/Cubemaps/HDRI/SampleBrdf.dds";

            env_SRV = Texture::InitAsDDSTexture(env_name, true, command_list);
            irradiance_SRV =
                Texture::InitAsDDSTexture(specular_name, true, command_list);
            specular_SRV =
                Texture::InitAsDDSTexture(irradiance_name, true, command_list);
            brdf_SRV = Texture::InitAsDDSTexture(brdf_name, true, command_list);

            D3D12_DESCRIPTOR_HEAP_DESC desc = {
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 7,
                D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE};

            dx12::ThrowIfFailed(
                dx12::GpuCore::Instance().device->CreateDescriptorHeap(
                    &desc, IID_PPV_ARGS(&condition->skybox_heap)));
            CD3DX12_CPU_DESCRIPTOR_HANDLE handle(
                condition->skybox_heap->GetCPUDescriptorHandleForHeapStart());

            UINT descriptor_size =
                dx12::GpuCore::Instance()
                    .device->GetDescriptorHandleIncrementSize(
                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            // t10~t12
            {
                auto resources = std::vector<ID3D12Resource *>{
                    env_SRV.Get(), irradiance_SRV.Get(), specular_SRV.Get()};

                D3D12_SHADER_RESOURCE_VIEW_DESC desc_SRV = {
                    DXGI_FORMAT_UNKNOWN, D3D12_SRV_DIMENSION_TEXTURECUBE,
                    D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING};

                for (auto resource : resources) {
                    if (resource != nullptr) {
                        desc_SRV.Format = resource->GetDesc().Format;
                        desc_SRV.TextureCube.MipLevels =
                            resource->GetDesc().MipLevels;
                        desc_SRV.TextureCube.MostDetailedMip = 0;
                        desc_SRV.TextureCube.ResourceMinLODClamp = 0;
                    }

                    dx12::GpuCore::Instance().device->CreateShaderResourceView(
                        resource, &desc_SRV, handle);
                    handle.Offset(descriptor_size);
                }
            }
            // t13
            {
                auto resources = std::vector<ID3D12Resource *>{brdf_SRV.Get()};

                D3D12_SHADER_RESOURCE_VIEW_DESC desc_SRV = {
                    DXGI_FORMAT_UNKNOWN, D3D12_SRV_DIMENSION_TEXTURE2D,
                    D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING};

                for (auto resource : resources) {
                    if (resource != nullptr) {
                        desc_SRV.Format = resource->GetDesc().Format;
                        desc_SRV.Texture2D.MipLevels =
                            resource->GetDesc().MipLevels;
                    }

                    dx12::GpuCore::Instance().device->CreateShaderResourceView(
                        resource, &desc_SRV, handle);
                    handle.Offset(descriptor_size);
                }
            }
            // t14~t16
            {
                D3D12_SHADER_RESOURCE_VIEW_DESC desc_SRV = {
                    DXGI_FORMAT_UNKNOWN, D3D12_SRV_DIMENSION_TEXTURE2D,
                    D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING};

                for (int i = 0; i < MAX_LIGHTS; i++) {
                    MakeTexture(1024, 1024, DXGI_FORMAT_R8G8B8A8_UNORM,
                                shadow_SRV[i]);
                    if (shadow_SRV[i] != nullptr) {
                        desc_SRV.Format = shadow_SRV[i]->GetDesc().Format;
                        desc_SRV.Texture2D.MipLevels =
                            shadow_SRV[i]->GetDesc().MipLevels;
                    }

                    dx12::GpuCore::Instance().device->CreateShaderResourceView(
                        shadow_SRV[i].Get(), &desc_SRV, handle);
                    handle.Offset(descriptor_size);
                }
            }

            break;
        }
        case EnumStageType::eRender: {

            auto renderer = (MeshRenderer *)skybox->GetComponent(
                EnumComponentType::eRenderer);
            auto mesh = renderer->meshes.front();

            skyboxPSO->Render(
                command_list, dx12::GpuCore::Instance().GetHandleHDR(),
                dx12::GpuCore::Instance().GetHandleDSV(),
                condition->skybox_heap, condition->sampler_heap,
                condition->global_consts.Get(), renderer->mesh_consts.Get(),
                renderer->material_consts.Get(), mesh->vertex_buffer_view,
                mesh->index_buffer_view, mesh->index_count);
            break;
        }
        default:
            break;
        }

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }

    void MakeTexture(int width, int height, DXGI_FORMAT format,
                     ComPtr<ID3D12Resource> &texture) {

        D3D12_RESOURCE_DESC txtDesc;
        ZeroMemory(&txtDesc, sizeof(txtDesc));
        txtDesc.Width = width;
        txtDesc.Height = height;
        txtDesc.MipLevels = 0;
        txtDesc.DepthOrArraySize = 1;
        txtDesc.Format = format;
        txtDesc.SampleDesc.Count = 1;
        txtDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        txtDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
        HRESULT hr = dx12::GpuCore::Instance().device->CreateCommittedResource(
            &heapProperties, D3D12_HEAP_FLAG_NONE, &txtDesc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr,
            IID_PPV_ARGS(texture.GetAddressOf()));
    }

    std::shared_ptr<dx12::SkyboxPSO> skyboxPSO;
    std::shared_ptr<Model> skybox;
    ComPtr<ID3D12Resource> env_SRV;
    ComPtr<ID3D12Resource> irradiance_SRV;
    ComPtr<ID3D12Resource> specular_SRV;
    ComPtr<ID3D12Resource> brdf_SRV;
    ComPtr<ID3D12Resource> shadow_SRV[MAX_LIGHTS];
};
} // namespace core

#endif
