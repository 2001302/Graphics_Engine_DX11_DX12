#ifndef _MESH_PSO
#define _MESH_PSO

#include "../../component/mesh_renderer.h"
#include "../../component/mirror_renderer.h"
#include "pipeline_state_object.h"
#include "../../util/shader_util.h"
#include "../../component/shadow_map.h"
#include "../../component/skybox_renderer.h"

namespace graphics {
class SolidMeshPSO : public GraphicsPSO {
  public:
    void Initialize() override {
        // s0 ~ s6
        CD3DX12_DESCRIPTOR_RANGE1 sampler_range;
        sampler_range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 7, 0);
        // t10 ~ t16
        CD3DX12_DESCRIPTOR_RANGE1 cubemap_range;
        cubemap_range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 10);
        CD3DX12_DESCRIPTOR_RANGE1 shadow_range;
        shadow_range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MAX_LIGHTS, 14);

        CD3DX12_ROOT_PARAMETER1 root_parameters[8] = {};

        root_parameters[0].InitAsDescriptorTable(1, &sampler_range,
                                                 D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[1].InitAsDescriptorTable(1, &cubemap_range,
                                                 D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[2].InitAsDescriptorTable(1, &shadow_range,
                                                 D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[3].InitAsConstantBufferView(
            0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[4].InitAsConstantBufferView(
            1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[5].InitAsConstantBufferView(
            2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            D3D12_SHADER_VISIBILITY_ALL);
        // VS : t0
        CD3DX12_DESCRIPTOR_RANGE1 texture_range_VS;
        texture_range_VS.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        root_parameters[6].InitAsDescriptorTable(
            1, &texture_range_VS, D3D12_SHADER_VISIBILITY_VERTEX);
        // PS : t0 ~ t4
        CD3DX12_DESCRIPTOR_RANGE1 texture_range_PS;
        texture_range_PS.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0);
        root_parameters[7].InitAsDescriptorTable(1, &texture_range_PS,
                                                 D3D12_SHADER_VISIBILITY_PIXEL);

        auto rootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(
            ARRAYSIZE(root_parameters), root_parameters, 0, nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        HRESULT hr = D3D12SerializeVersionedRootSignature(&rootSignatureDesc,
                                                          &signature, &error);

        hr = GpuCore::Instance().GetDevice()->CreateRootSignature(
            0, signature->GetBufferPointer(), signature->GetBufferSize(),
            IID_PPV_ARGS(&root_signature));

        // shader
        ComPtr<ID3DBlob> basicVS;
        ComPtr<ID3DBlob> basicPS;
        ShaderUtil::CreateVertexShader(GpuCore::Instance().GetDevice(),
                                       L"../DX12/Shader/BasicVS.hlsl", basicVS);
        ShaderUtil::CreatePixelShader(GpuCore::Instance().GetDevice(),
                                      L"../DX12/Shader/BasicPS.hlsl", basicPS);

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {layout::basicIEs, _countof(layout::basicIEs)};
        psoDesc.pRootSignature = root_signature;
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(basicVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(rasterizer::solidRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState = depth::drawDSS;
        psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleDesc.Quality = 0;

        ASSERT_FAILED(
            GpuCore::Instance().GetDevice()->CreateGraphicsPipelineState(
                &psoDesc, IID_PPV_ARGS(&pipeline_state)));
    };
    void Render(common::Model *world, SamplerState *sampler_state,
                ComPtr<ID3D12Resource> global_consts,
                MeshRenderer *mesh_renderer) {

        auto context =
            GpuCore::Instance().GetCommand()->Begin<GraphicsCommandContext>(
                L"MeshRenderer");

        context->SetDescriptorHeaps(
            std::vector{GpuCore::Instance().GetHeap().View(),
                        GpuCore::Instance().GetHeap().Sampler()});

        context->TransitionResource(GpuBuffer::Instance().GetHDR(),
                                    D3D12_RESOURCE_STATE_RENDER_TARGET, true);

        for (auto mesh : mesh_renderer->GetMeshes()) {

            context->SetViewportAndScissorRect(
                0, 0, (UINT)common::env::screen_width,
                (UINT)common::env::screen_height);
            context->SetRenderTargetView(
                GpuBuffer::Instance().GetHDR()->GetRtvHandle(),
                GpuBuffer::Instance().GetDSV()->GetDsvHandle());

            context->SetRootSignature(root_signature);
            context->SetPipelineState(pipeline_state);

            context->GetList()->SetGraphicsRootDescriptorTable(
                0, sampler_state->GetGpuHandle());
            // global texture
            context->GetList()->SetGraphicsRootDescriptorTable(
                1, SkyboxRenderer::GetSkyboxTexture(world)->GetGpuHandle());
            context->GetList()->SetGraphicsRootDescriptorTable(
                2, ShadowMap::GetShadowMap(world)->GetGpuHandle());
            context->GetList()->SetGraphicsRootConstantBufferView(
                3, global_consts->GetGPUVirtualAddress());
            context->GetList()->SetGraphicsRootConstantBufferView(
                4, mesh_renderer->MeshConsts().Get()->GetGPUVirtualAddress());
            context->GetList()->SetGraphicsRootConstantBufferView(
                5,
                mesh_renderer->MaterialConsts().Get()->GetGPUVirtualAddress());
            context->GetList()->SetGraphicsRootDescriptorTable(
                6, mesh->texture_VS->GetGpuHandle());
            context->GetList()->SetGraphicsRootDescriptorTable(
                7, mesh->texture_PS->GetGpuHandle());

            context->GetList()->IASetPrimitiveTopology(
                D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            context->GetList()->IASetVertexBuffers(0, 1,
                                                   &mesh->vertex_buffer_view);
            context->GetList()->IASetIndexBuffer(&mesh->index_buffer_view);
            context->GetList()->DrawIndexedInstanced(mesh->index_count, 1, 0, 0,
                                                     0);
        }

        GpuCore::Instance().GetCommand()->Finish(context);
    };
};
class ReflectSolidMeshPSO : public GraphicsPSO {
  public:
    void Initialize() override {
        // s0 ~ s6
        CD3DX12_DESCRIPTOR_RANGE1 sampler_range;
        sampler_range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 7, 0);
        // t10 ~ t16
        CD3DX12_DESCRIPTOR_RANGE1 cubemap_range;
        cubemap_range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 10);
        CD3DX12_DESCRIPTOR_RANGE1 shadow_range;
        shadow_range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MAX_LIGHTS, 14);

        CD3DX12_ROOT_PARAMETER1 root_parameters[8] = {};

        root_parameters[0].InitAsDescriptorTable(1, &sampler_range,
                                                 D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[1].InitAsDescriptorTable(1, &cubemap_range,
                                                 D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[2].InitAsDescriptorTable(1, &shadow_range,
                                                 D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[3].InitAsConstantBufferView(
            0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[4].InitAsConstantBufferView(
            1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[5].InitAsConstantBufferView(
            2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            D3D12_SHADER_VISIBILITY_ALL);
        // VS : t0
        CD3DX12_DESCRIPTOR_RANGE1 texture_range_VS;
        texture_range_VS.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        root_parameters[6].InitAsDescriptorTable(
            1, &texture_range_VS, D3D12_SHADER_VISIBILITY_VERTEX);
        // PS : t0 ~ t4
        CD3DX12_DESCRIPTOR_RANGE1 texture_range_PS;
        texture_range_PS.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0);
        root_parameters[7].InitAsDescriptorTable(1, &texture_range_PS,
                                                 D3D12_SHADER_VISIBILITY_PIXEL);

        auto rootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(
            ARRAYSIZE(root_parameters), root_parameters, 0, nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        HRESULT hr = D3D12SerializeVersionedRootSignature(&rootSignatureDesc,
                                                          &signature, &error);

        hr = GpuCore::Instance().GetDevice()->CreateRootSignature(
            0, signature->GetBufferPointer(), signature->GetBufferSize(),
            IID_PPV_ARGS(&root_signature));

        // shader
        ComPtr<ID3DBlob> basicVS;
        ComPtr<ID3DBlob> basicPS;
        ShaderUtil::CreateVertexShader(GpuCore::Instance().GetDevice(),
                                       L"../DX12/Shader/BasicVS.hlsl", basicVS);
        ShaderUtil::CreatePixelShader(GpuCore::Instance().GetDevice(),
                                      L"../DX12/Shader/BasicPS.hlsl", basicPS);
        //
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {layout::basicIEs, _countof(layout::basicIEs)};
        psoDesc.pRootSignature = root_signature;
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(basicVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
        psoDesc.RasterizerState =
            CD3DX12_RASTERIZER_DESC(rasterizer::solidCCWRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState = depth::drawMaskedDSS;
        psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleDesc.Quality = 0;

        ASSERT_FAILED(
            GpuCore::Instance().GetDevice()->CreateGraphicsPipelineState(
                &psoDesc, IID_PPV_ARGS(&pipeline_state)));
    };
    void Render(common::Model *world, SamplerState *sampler_state,
                ComPtr<ID3D12Resource> global_consts,
                MeshRenderer *mesh_renderer) {

        auto context =
            GpuCore::Instance().GetCommand()->Begin<GraphicsCommandContext>(
                L"ReflectMeshRenderer");

        context->SetDescriptorHeaps(
            std::vector{GpuCore::Instance().GetHeap().View(),
                        GpuCore::Instance().GetHeap().Sampler()});

        context->TransitionResource(GpuBuffer::Instance().GetHDR(),
                                    D3D12_RESOURCE_STATE_RENDER_TARGET, true);

        for (auto mesh : mesh_renderer->GetMeshes()) {

            context->SetViewportAndScissorRect(
                0, 0, (UINT)common::env::screen_width,
                (UINT)common::env::screen_height);
            context->SetRenderTargetView(
                GpuBuffer::Instance().GetHDR()->GetRtvHandle(),
                GpuBuffer::Instance().GetDSV()->GetDsvHandle());

            context->SetRootSignature(root_signature);
            context->SetPipelineState(pipeline_state);

            context->GetList()->OMSetStencilRef(1);
            context->GetList()->SetGraphicsRootDescriptorTable(
                0, sampler_state->GetGpuHandle());
            // global texture
            context->GetList()->SetGraphicsRootDescriptorTable(
                1, SkyboxRenderer::GetSkyboxTexture(world)->GetGpuHandle());
            context->GetList()->SetGraphicsRootDescriptorTable(
                2, ShadowMap::GetShadowMap(world)->GetGpuHandle());
            context->GetList()->SetGraphicsRootConstantBufferView(
                3, global_consts->GetGPUVirtualAddress());
            context->GetList()->SetGraphicsRootConstantBufferView(
                4, mesh_renderer->MeshConsts().Get()->GetGPUVirtualAddress());
            context->GetList()->SetGraphicsRootConstantBufferView(
                5,
                mesh_renderer->MaterialConsts().Get()->GetGPUVirtualAddress());
            context->GetList()->SetGraphicsRootDescriptorTable(
                6, mesh->texture_VS->GetGpuHandle());
            context->GetList()->SetGraphicsRootDescriptorTable(
                7, mesh->texture_PS->GetGpuHandle());

            context->GetList()->IASetPrimitiveTopology(
                D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            context->GetList()->IASetVertexBuffers(0, 1,
                                                   &mesh->vertex_buffer_view);
            context->GetList()->IASetIndexBuffer(&mesh->index_buffer_view);
            context->GetList()->DrawIndexedInstanced(mesh->index_count, 1, 0, 0,
                                                     0);
        }

        GpuCore::Instance().GetCommand()->Finish(context);
    };
};
class MirrorBlendSolidMeshPSO : public GraphicsPSO {
  public:
    void Initialize() override {
        // rootSignature
        // s0 ~ s6
        CD3DX12_DESCRIPTOR_RANGE1 sampler_range;
        sampler_range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 7, 0);

        // t10 ~ t16
        CD3DX12_DESCRIPTOR_RANGE1 cubemap_range;
        cubemap_range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 10);
        CD3DX12_DESCRIPTOR_RANGE1 shadow_range;
        shadow_range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MAX_LIGHTS, 14);

        CD3DX12_ROOT_PARAMETER1 root_parameters[8] = {};

        root_parameters[0].InitAsDescriptorTable(1, &sampler_range,
                                                 D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[1].InitAsDescriptorTable(1, &cubemap_range,
                                                 D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[2].InitAsDescriptorTable(1, &shadow_range,
                                                 D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[3].InitAsConstantBufferView(
            0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[4].InitAsConstantBufferView(
            1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[5].InitAsConstantBufferView(
            2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            D3D12_SHADER_VISIBILITY_ALL);
        // VS : t0
        CD3DX12_DESCRIPTOR_RANGE1 texture_range_VS;
        texture_range_VS.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        root_parameters[6].InitAsDescriptorTable(
            1, &texture_range_VS, D3D12_SHADER_VISIBILITY_VERTEX);
        // PS : t0 ~ t4
        CD3DX12_DESCRIPTOR_RANGE1 texture_range_PS;
        texture_range_PS.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0);
        root_parameters[7].InitAsDescriptorTable(1, &texture_range_PS,
                                                 D3D12_SHADER_VISIBILITY_PIXEL);

        auto rootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(
            ARRAYSIZE(root_parameters), root_parameters, 0, nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        HRESULT hr = D3D12SerializeVersionedRootSignature(&rootSignatureDesc,
                                                          &signature, &error);

        hr = GpuCore::Instance().GetDevice()->CreateRootSignature(
            0, signature->GetBufferPointer(), signature->GetBufferSize(),
            IID_PPV_ARGS(&root_signature));

        // shader
        ComPtr<ID3DBlob> basicVS;
        ComPtr<ID3DBlob> basicPS;
        ShaderUtil::CreateVertexShader(GpuCore::Instance().GetDevice(),
                                       L"../DX12/Shader/BasicVS.hlsl", basicVS);
        ShaderUtil::CreatePixelShader(GpuCore::Instance().GetDevice(),
                                      L"../DX12/Shader/BasicPS.hlsl", basicPS);
        //
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {layout::basicIEs, _countof(layout::basicIEs)};
        psoDesc.pRootSignature = root_signature;
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(basicVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(rasterizer::solidRS);
        psoDesc.BlendState = blend::mirrorBS;
        psoDesc.DepthStencilState = depth::drawMaskedDSS;
        psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleDesc.Quality = 0;

        ASSERT_FAILED(
            GpuCore::Instance().GetDevice()->CreateGraphicsPipelineState(
                &psoDesc, IID_PPV_ARGS(&pipeline_state)));
    };
    void Render(common::Model *world, SamplerState *sampler_state,
                ComPtr<ID3D12Resource> global_consts,
                MirrorRenderer *mesh_renderer) {

        auto context =
            GpuCore::Instance().GetCommand()->Begin<GraphicsCommandContext>(
                L"MirrorBlendMeshRenderer");

        context->SetDescriptorHeaps(
            std::vector{GpuCore::Instance().GetHeap().View(),
                        GpuCore::Instance().GetHeap().Sampler()});

        context->TransitionResource(GpuBuffer::Instance().GetHDR(),
                                    D3D12_RESOURCE_STATE_RENDER_TARGET, true);

        float alpha = mesh_renderer->GetMirrorAlpha();
        float blend_factor[4] = {alpha, alpha, alpha, 1.0f};

        for (auto mesh : mesh_renderer->GetMeshes()) {
            context->SetViewportAndScissorRect(
                0, 0, (UINT)common::env::screen_width,
                (UINT)common::env::screen_height);
            context->SetRenderTargetView(
                GpuBuffer::Instance().GetHDR()->GetRtvHandle(),
                GpuBuffer::Instance().GetDSV()->GetDsvHandle());

            context->SetRootSignature(root_signature);
            context->SetPipelineState(pipeline_state);
            context->GetList()->OMSetStencilRef(1);
            context->GetList()->OMSetBlendFactor(blend_factor);
            context->GetList()->SetGraphicsRootDescriptorTable(
                0, sampler_state->GetGpuHandle());
            // global texture
            context->GetList()->SetGraphicsRootDescriptorTable(
                1, SkyboxRenderer::GetSkyboxTexture(world)->GetGpuHandle());
            context->GetList()->SetGraphicsRootDescriptorTable(
                2, ShadowMap::GetShadowMap(world)->GetGpuHandle());
            context->GetList()->SetGraphicsRootConstantBufferView(
                3, global_consts->GetGPUVirtualAddress());
            context->GetList()->SetGraphicsRootConstantBufferView(
                4, mesh_renderer->MeshConsts().Get()->GetGPUVirtualAddress());
            context->GetList()->SetGraphicsRootConstantBufferView(
                5,
                mesh_renderer->MaterialConsts().Get()->GetGPUVirtualAddress());
            context->GetList()->SetGraphicsRootDescriptorTable(
                6, mesh->texture_VS->GetGpuHandle());
            context->GetList()->SetGraphicsRootDescriptorTable(
                7, mesh->texture_PS->GetGpuHandle());

            context->GetList()->IASetPrimitiveTopology(
                D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            context->GetList()->IASetVertexBuffers(0, 1,
                                                   &mesh->vertex_buffer_view);
            context->GetList()->IASetIndexBuffer(&mesh->index_buffer_view);
            context->GetList()->DrawIndexedInstanced(mesh->index_count, 1, 0, 0,
                                                     0);
        }

        GpuCore::Instance().GetCommand()->Finish(context);
    };
};
class SkinnedSolidMeshPSO : public GraphicsPSO {
  public:
    void Initialize() override {
        // s0 ~ s6
        CD3DX12_DESCRIPTOR_RANGE1 sampler_range;
        sampler_range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 7, 0);

        // t10 ~ t16
        CD3DX12_DESCRIPTOR_RANGE1 cubemap_range;
        cubemap_range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 10);
        CD3DX12_DESCRIPTOR_RANGE1 shadow_range;
        shadow_range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MAX_LIGHTS, 14);

        CD3DX12_DESCRIPTOR_RANGE1 boneRange;
        boneRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 9);

        CD3DX12_ROOT_PARAMETER1 root_parameters[9] = {};

        root_parameters[0].InitAsDescriptorTable(1, &sampler_range,
                                                 D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[1].InitAsDescriptorTable(1, &cubemap_range,
                                                 D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[2].InitAsDescriptorTable(1, &shadow_range,
                                                 D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[3].InitAsConstantBufferView(
            0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[4].InitAsConstantBufferView(
            1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[5].InitAsConstantBufferView(
            2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            D3D12_SHADER_VISIBILITY_ALL);
        // VS : t0
        CD3DX12_DESCRIPTOR_RANGE1 texture_range_VS;
        texture_range_VS.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        root_parameters[6].InitAsDescriptorTable(
            1, &texture_range_VS, D3D12_SHADER_VISIBILITY_VERTEX);
        // PS : t0 ~ t4
        CD3DX12_DESCRIPTOR_RANGE1 texture_range_PS;
        texture_range_PS.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0);
        root_parameters[7].InitAsDescriptorTable(1, &texture_range_PS,
                                                 D3D12_SHADER_VISIBILITY_PIXEL);
        root_parameters[8].InitAsDescriptorTable(1, &boneRange,
                                                 D3D12_SHADER_VISIBILITY_ALL);

        auto rootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(
            ARRAYSIZE(root_parameters), root_parameters, 0, nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        HRESULT hr = D3D12SerializeVersionedRootSignature(&rootSignatureDesc,
                                                          &signature, &error);

        hr = GpuCore::Instance().GetDevice()->CreateRootSignature(
            0, signature->GetBufferPointer(), signature->GetBufferSize(),
            IID_PPV_ARGS(&root_signature));

        // shader
        ComPtr<ID3DBlob> basicVS;
        ComPtr<ID3DBlob> basicPS;
        ShaderUtil::CreateVertexShader(
            GpuCore::Instance().GetDevice(), L"../DX12/Shader/BasicVS.hlsl",
            basicVS,
            std::vector<D3D_SHADER_MACRO>{{"SKINNED", "1"}, {NULL, NULL}});
        ShaderUtil::CreatePixelShader(GpuCore::Instance().GetDevice(),
                                      L"../DX12/Shader/BasicPS.hlsl", basicPS);
        //
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {layout::skinnedIEs,
                               _countof(layout::skinnedIEs)};
        psoDesc.pRootSignature = root_signature;
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(basicVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(rasterizer::solidRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState = depth::drawDSS;
        psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleDesc.Quality = 0;

        ASSERT_FAILED(
            GpuCore::Instance().GetDevice()->CreateGraphicsPipelineState(
                &psoDesc, IID_PPV_ARGS(&pipeline_state)));
    };
    void Render(common::Model *world, SamplerState *sampler_state,
                ComPtr<ID3D12Resource> global_consts,
                MeshRenderer *mesh_renderer, Animator *animator) {

        auto context =
            GpuCore::Instance().GetCommand()->Begin<GraphicsCommandContext>(
                L"MeshRenderer");

        context->SetDescriptorHeaps(
            std::vector{GpuCore::Instance().GetHeap().View(),
                        GpuCore::Instance().GetHeap().Sampler()});

        context->TransitionResource(GpuBuffer::Instance().GetHDR(),
                                    D3D12_RESOURCE_STATE_RENDER_TARGET, true);

        for (auto mesh : mesh_renderer->GetMeshes()) {

            context->SetViewportAndScissorRect(
                0, 0, (UINT)common::env::screen_width,
                (UINT)common::env::screen_height);
            context->SetRenderTargetView(
                GpuBuffer::Instance().GetHDR()->GetRtvHandle(),
                GpuBuffer::Instance().GetDSV()->GetDsvHandle());

            context->SetRootSignature(root_signature);
            context->SetPipelineState(pipeline_state);

            context->GetList()->SetGraphicsRootDescriptorTable(
                0, sampler_state->GetGpuHandle());
            // global texture
            context->GetList()->SetGraphicsRootDescriptorTable(
                1, SkyboxRenderer::GetSkyboxTexture(world)->GetGpuHandle());
            context->GetList()->SetGraphicsRootDescriptorTable(
                2, ShadowMap::GetShadowMap(world)->GetGpuHandle());
            context->GetList()->SetGraphicsRootConstantBufferView(
                3, global_consts->GetGPUVirtualAddress());
            context->GetList()->SetGraphicsRootConstantBufferView(
                4, mesh_renderer->MeshConsts().Get()->GetGPUVirtualAddress());
            context->GetList()->SetGraphicsRootConstantBufferView(
                5,
                mesh_renderer->MaterialConsts().Get()->GetGPUVirtualAddress());
            context->GetList()->SetGraphicsRootDescriptorTable(
                6, mesh->texture_VS->GetGpuHandle());
            context->GetList()->SetGraphicsRootDescriptorTable(
                7, mesh->texture_PS->GetGpuHandle());
            context->GetList()->SetGraphicsRootDescriptorTable(
                8, animator->GetGpuHandle());

            context->GetList()->IASetPrimitiveTopology(
                D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            context->GetList()->IASetVertexBuffers(0, 1,
                                                   &mesh->vertex_buffer_view);
            context->GetList()->IASetIndexBuffer(&mesh->index_buffer_view);
            context->GetList()->DrawIndexedInstanced(mesh->index_count, 1, 0, 0,
                                                     0);
        }

        GpuCore::Instance().GetCommand()->Finish(context);
    };
};
class ReflectSkinnedSolidMeshPSO : public GraphicsPSO {
  public:
    void Initialize() override {
        // s0 ~ s6
        CD3DX12_DESCRIPTOR_RANGE1 sampler_range;
        sampler_range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 7, 0);
        // t10 ~ t16
        CD3DX12_DESCRIPTOR_RANGE1 cubemap_range;
        cubemap_range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 10);
        CD3DX12_DESCRIPTOR_RANGE1 shadow_range;
        shadow_range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MAX_LIGHTS, 14);
        CD3DX12_DESCRIPTOR_RANGE1 bone_range;
        bone_range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 9);

        CD3DX12_ROOT_PARAMETER1 root_parameters[9] = {};

        root_parameters[0].InitAsDescriptorTable(1, &sampler_range,
                                                 D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[1].InitAsDescriptorTable(1, &cubemap_range,
                                                 D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[2].InitAsDescriptorTable(1, &shadow_range,
                                                 D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[3].InitAsConstantBufferView(
            0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[4].InitAsConstantBufferView(
            1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[5].InitAsConstantBufferView(
            2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            D3D12_SHADER_VISIBILITY_ALL);
        // VS : t0
        CD3DX12_DESCRIPTOR_RANGE1 texture_range_VS;
        texture_range_VS.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        root_parameters[6].InitAsDescriptorTable(
            1, &texture_range_VS, D3D12_SHADER_VISIBILITY_VERTEX);
        // PS : t0 ~ t4
        CD3DX12_DESCRIPTOR_RANGE1 texture_range_PS;
        texture_range_PS.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0);
        root_parameters[7].InitAsDescriptorTable(1, &texture_range_PS,
                                                 D3D12_SHADER_VISIBILITY_PIXEL);
        root_parameters[8].InitAsDescriptorTable(1, &bone_range,
                                                 D3D12_SHADER_VISIBILITY_ALL);

        auto rootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(
            ARRAYSIZE(root_parameters), root_parameters, 0, nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        HRESULT hr = D3D12SerializeVersionedRootSignature(&rootSignatureDesc,
                                                          &signature, &error);

        hr = GpuCore::Instance().GetDevice()->CreateRootSignature(
            0, signature->GetBufferPointer(), signature->GetBufferSize(),
            IID_PPV_ARGS(&root_signature));

        // shader
        ComPtr<ID3DBlob> basicVS;
        ComPtr<ID3DBlob> basicPS;
        ShaderUtil::CreateVertexShader(
            GpuCore::Instance().GetDevice(), L"../DX12/Shader/BasicVS.hlsl",
            basicVS,
            std::vector<D3D_SHADER_MACRO>{{"SKINNED", "1"}, {NULL, NULL}});
        ShaderUtil::CreatePixelShader(GpuCore::Instance().GetDevice(),
                                      L"../DX12/Shader/BasicPS.hlsl", basicPS);
        //
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {layout::skinnedIEs,
                               _countof(layout::skinnedIEs)};
        psoDesc.pRootSignature = root_signature;
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(basicVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
        psoDesc.RasterizerState =
            CD3DX12_RASTERIZER_DESC(rasterizer::solidCCWRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState = depth::drawMaskedDSS;
        psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleDesc.Quality = 0;

        ASSERT_FAILED(
            GpuCore::Instance().GetDevice()->CreateGraphicsPipelineState(
                &psoDesc, IID_PPV_ARGS(&pipeline_state)));
    };
    void Render(common::Model *world, SamplerState *sampler_state,
                ComPtr<ID3D12Resource> global_consts,
                MeshRenderer *mesh_renderer, Animator *animator) {

        auto context =
            GpuCore::Instance().GetCommand()->Begin<GraphicsCommandContext>(
                L"ReflectMeshRenderer");

        context->SetDescriptorHeaps(
            std::vector{GpuCore::Instance().GetHeap().View(),
                        GpuCore::Instance().GetHeap().Sampler()});

        context->TransitionResource(GpuBuffer::Instance().GetHDR(),
                                    D3D12_RESOURCE_STATE_RENDER_TARGET, true);

        for (auto mesh : mesh_renderer->GetMeshes()) {

            context->SetViewportAndScissorRect(
                0, 0, (UINT)common::env::screen_width,
                (UINT)common::env::screen_height);
            context->SetRenderTargetView(
                GpuBuffer::Instance().GetHDR()->GetRtvHandle(),
                GpuBuffer::Instance().GetDSV()->GetDsvHandle());

            context->SetRootSignature(root_signature);
            context->SetPipelineState(pipeline_state);

            context->GetList()->OMSetStencilRef(1);
            context->GetList()->SetGraphicsRootDescriptorTable(
                0, sampler_state->GetGpuHandle());
            // global texture
            context->GetList()->SetGraphicsRootDescriptorTable(
                1, SkyboxRenderer::GetSkyboxTexture(world)->GetGpuHandle());
            context->GetList()->SetGraphicsRootDescriptorTable(
                2, ShadowMap::GetShadowMap(world)->GetGpuHandle());
            context->GetList()->SetGraphicsRootConstantBufferView(
                3, global_consts->GetGPUVirtualAddress());
            context->GetList()->SetGraphicsRootConstantBufferView(
                4, mesh_renderer->MeshConsts().Get()->GetGPUVirtualAddress());
            context->GetList()->SetGraphicsRootConstantBufferView(
                5,
                mesh_renderer->MaterialConsts().Get()->GetGPUVirtualAddress());
            context->GetList()->SetGraphicsRootDescriptorTable(
                6, mesh->texture_VS->GetGpuHandle());
            context->GetList()->SetGraphicsRootDescriptorTable(
                7, mesh->texture_PS->GetGpuHandle());
            context->GetList()->SetGraphicsRootDescriptorTable(
                8, animator->GetGpuHandle());

            context->GetList()->IASetPrimitiveTopology(
                D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            context->GetList()->IASetVertexBuffers(0, 1,
                                                   &mesh->vertex_buffer_view);
            context->GetList()->IASetIndexBuffer(&mesh->index_buffer_view);
            context->GetList()->DrawIndexedInstanced(mesh->index_count, 1, 0, 0,
                                                     0);
        }

        GpuCore::Instance().GetCommand()->Finish(context);
    };
};
} // namespace graphics
#endif