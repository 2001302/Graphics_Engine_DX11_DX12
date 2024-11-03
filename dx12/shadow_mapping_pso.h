#ifndef _SHADOW_MAPPING_PSO
#define _SHADOW_MAPPING_PSO

#include "mesh_renderer.h"
#include "pipeline_state_object.h"
#include "shader_util.h"
#include "skybox_renderer.h"

namespace graphics {
class ShadowMappingPSO : public GraphicsPSO {
  public:
    void Initialize() override {
        // rootSignature
        // s0 ~ s6
        CD3DX12_DESCRIPTOR_RANGE1 sampler_range;
        sampler_range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 7, 0);

        // t10 ~ t13
        CD3DX12_DESCRIPTOR_RANGE1 textureRange;
        textureRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 10);

        CD3DX12_ROOT_PARAMETER1 root_parameters[5] = {};
        // Common.hlsli : s0~s6,t10~t16,b0~b2
        root_parameters[0].InitAsDescriptorTable(1, &sampler_range,
                                                 D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[1].InitAsDescriptorTable(1, &textureRange,
                                                 D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[2].InitAsConstantBufferView(
            0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[3].InitAsConstantBufferView(
            1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[4].InitAsConstantBufferView(
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
        psoDesc.DepthStencilState = depth::drawDSS;
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
    void Render(common::Model *skybox, SamplerState *sampler_state,
                ComPtr<ID3D12Resource> global_consts,
                std::map<int /*id*/, std::shared_ptr<common::Model>> objects,
                DepthBuffer *depth_buffer) {

        auto context =
            GpuCore::Instance().GetCommand()->Begin<GraphicsCommandContext>(
                L"ShadowMapping");
        context->SetDescriptorHeaps(
            std::vector{GpuCore::Instance().GetHeap().View(),
                        GpuCore::Instance().GetHeap().Sampler()});
        context->TransitionResource(depth_buffer,
                                    D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
        context->ClearDepthStencilView(depth_buffer->GetDsvHandle(),
                                       D3D12_CLEAR_FLAG_DEPTH);

        for (auto &object : objects) {
            MeshRenderer *renderer = nullptr;
            object.second->TryGet(renderer);

            context->SetViewportAndScissorRect(0, 0,
                                               (UINT)depth_buffer->GetWidth(),
                                               (UINT)depth_buffer->GetHeight());
            context->SetDepthStencilView(depth_buffer->GetDsvHandle());

            context->SetRootSignature(root_signature);
            context->SetPipelineState(pipeline_state);

            for (auto mesh : renderer->GetMeshes()) {

                context->GetList()->SetGraphicsRootDescriptorTable(
                    0, sampler_state->GetGpuHandle());
                context->GetList()->SetGraphicsRootDescriptorTable(
                    1,
                    SkyboxRenderer::GetSkyboxTexture(skybox)->GetGpuHandle());
                context->GetList()->SetGraphicsRootConstantBufferView(
                    2, global_consts->GetGPUVirtualAddress());
                context->GetList()->SetGraphicsRootConstantBufferView(
                    3, renderer->MeshConsts().Get()->GetGPUVirtualAddress());
                context->GetList()->SetGraphicsRootConstantBufferView(
                    4,
                    renderer->MaterialConsts().Get()->GetGPUVirtualAddress());

                context->GetList()->IASetPrimitiveTopology(
                    D3D12_PRIMITIVE_TOPOLOGY::
                        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                context->GetList()->IASetVertexBuffers(
                    0, 1, &mesh->vertex_buffer_view);
                context->GetList()->IASetIndexBuffer(&mesh->index_buffer_view);
                context->GetList()->DrawIndexedInstanced(mesh->index_count, 1,
                                                         0, 0, 0);
            }
        }
        context->TransitionResource(
            depth_buffer,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE |
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            true);
        GpuCore::Instance().GetCommand()->Finish(context);
    };
};
} // namespace graphics

#endif