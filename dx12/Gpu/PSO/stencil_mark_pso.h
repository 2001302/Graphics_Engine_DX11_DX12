#ifndef _STENCIL_MARK_PSO
#define _STENCIL_MARK_PSO

#include "../../component/mesh_renderer.h"
#include "pipeline_state_object.h"
#include "../../util/shader_util.h"
#include "../../component/skybox_renderer.h"

namespace graphics {
class StencilMarkPSO : public GraphicsPSO {
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

        CD3DX12_ROOT_PARAMETER1 root_parameters[6] = {};
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
        ComPtr<ID3DBlob> depthVS;
        ComPtr<ID3DBlob> depthPS;
        ShaderUtil::CreateVertexShader(GpuCore::Instance().GetDevice(),
                                       L"../DX12/Shader/DepthOnlyVS.hlsl",
                                       depthVS);
        ShaderUtil::CreatePixelShader(GpuCore::Instance().GetDevice(),
                                      L"../DX12/Shader/DepthOnlyPS.hlsl",
                                      depthPS);
        //
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {layout::basicIEs, _countof(layout::basicIEs)};
        psoDesc.pRootSignature = root_signature;
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(depthVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(depthPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(rasterizer::solidRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState = depth::maskDSS;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
        psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
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
                L"StencilMark");

        context->SetDescriptorHeaps(
            std::vector{GpuCore::Instance().GetHeap().View(),
                        GpuCore::Instance().GetHeap().Sampler()});

        context->TransitionResource(GpuBuffer::Instance().GetHDR(),
                                    D3D12_RESOURCE_STATE_RENDER_TARGET, true);

        for (auto mesh : mesh_renderer->GetMeshes()) {

            context->SetViewportAndScissorRect(
                0, 0, (UINT)common::env::screen_width,
                (UINT)common::env::screen_height);
            context->SetDepthStencilView(
                GpuBuffer::Instance().GetDSV()->GetDsvHandle());
            context->GetList()->OMSetStencilRef(1);
            context->SetRootSignature(root_signature);
            context->SetPipelineState(pipeline_state);

            context->GetList()->SetGraphicsRootDescriptorTable(
                0, sampler_state->GetGpuHandle());
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

            context->GetList()->IASetPrimitiveTopology(
                D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            context->GetList()->IASetVertexBuffers(0, 1,
                                                   &mesh->vertex_buffer_view);
            context->GetList()->IASetIndexBuffer(&mesh->index_buffer_view);
            context->GetList()->DrawIndexedInstanced(mesh->index_count, 1, 0, 0,
                                                     0);
            context->ClearDepthStencilView(
                GpuBuffer::Instance().GetDSV()->GetDsvHandle(),
                D3D12_CLEAR_FLAG_DEPTH);
        }

        GpuCore::Instance().GetCommand()->Finish(context);
    };
};
} // namespace graphics

#endif