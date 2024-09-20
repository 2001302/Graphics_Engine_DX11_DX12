#ifndef _TONEMAPPING_PSO
#define _TONEMAPPING_PSO

#include "graphics_pso.h"
#include "shader_util.h"

namespace graphics {
class ToneMappingPSO : public GraphicsPSO {
  public:
    void Initialize() override {
        CD3DX12_DESCRIPTOR_RANGE1 samplerRange;
        samplerRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

        CD3DX12_DESCRIPTOR_RANGE1 srvRange;
        srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

        // rootSignature
        CD3DX12_ROOT_PARAMETER1 rootParameters[3] = {};
        // constant buffer
        rootParameters[0].InitAsConstantBufferView(
            1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            D3D12_SHADER_VISIBILITY_ALL);
        // sampler
        rootParameters[1].InitAsDescriptorTable(1, &samplerRange,
                                                D3D12_SHADER_VISIBILITY_ALL);
        // srv
        rootParameters[2].InitAsDescriptorTable(1, &srvRange,
                                                D3D12_SHADER_VISIBILITY_ALL);

        auto rootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(
            ARRAYSIZE(rootParameters), rootParameters, 0, nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ASSERT_FAILED(D3D12SerializeVersionedRootSignature(&rootSignatureDesc,
                                                           &signature, &error));

        ASSERT_FAILED(GpuCore::Instance().GetDevice()->CreateRootSignature(
            0, signature->GetBufferPointer(), signature->GetBufferSize(),
            IID_PPV_ARGS(&root_signature)));

        // shader
        ComPtr<ID3DBlob> tone_mappingVS;
        ShaderUtil::CreateVertexShader(GpuCore::Instance().GetDevice(),
                                       L"Shader//ToneMappingVS.hlsl",
                                       tone_mappingVS);
        ComPtr<ID3DBlob> tone_mappingPS;
        ShaderUtil::CreatePixelShader(GpuCore::Instance().GetDevice(),
                                      L"Shader//ToneMappingPS.hlsl",
                                      tone_mappingPS);
        // pipeline state
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {layout::combineIEs,
                               _countof(layout::combineIEs)};
        psoDesc.pRootSignature = root_signature;
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(tone_mappingVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(tone_mappingPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(rasterizer::postRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.StencilEnable = false;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleDesc.Quality = 0;

        ASSERT_FAILED(
            GpuCore::Instance().GetDevice()->CreateGraphicsPipelineState(
                &psoDesc, IID_PPV_ARGS(&pipeline_state)));
    };
    void Render(ComPtr<ID3D12Resource> const_buffer,
                D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view,
                D3D12_INDEX_BUFFER_VIEW index_buffer_view, UINT index_count,
                SamplerState sampler) {

        auto context =
            GpuCore::Instance().GetCommand()->Begin<GraphicsCommandContext>(
                L"ToneMapping");

        context->SetDescriptorHeaps(
            std::vector{GpuCore::Instance().GetHeap().View(),
                        GpuCore::Instance().GetHeap().Sampler()});

        context->TransitionResource(GpuBuffer::Instance().GetDisplay(),
                                    D3D12_RESOURCE_STATE_RENDER_TARGET, true);

        context->TransitionResource(
            GpuBuffer::Instance().GetHDR(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            true);

        context->SetViewportAndScissorRect(0, 0,
                                           (UINT)common::env::screen_width,
                                           (UINT)common::env::screen_height);

        context->SetRenderTargetView(GpuBuffer::Instance().GetDisplay());

        context->SetRootSignature(root_signature);
        context->SetPipelineState(pipeline_state);

        // 1:constant buffer
        context->GetList()->SetGraphicsRootConstantBufferView(
            0, const_buffer->GetGPUVirtualAddress());
        // 2:sampler
        context->GetList()->SetGraphicsRootDescriptorTable(
            1, sampler.GetGpuHandle());
        // 3:srv
        context->GetList()->SetGraphicsRootDescriptorTable(
            2, GpuBuffer::Instance().GetHDR()->GetSrvGpuHandle());

        context->GetList()->IASetPrimitiveTopology(
            D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->GetList()->IASetVertexBuffers(0, 1, &vertex_buffer_view);
        context->GetList()->IASetIndexBuffer(&index_buffer_view);
        context->GetList()->DrawIndexedInstanced(index_count, 1, 0, 0, 0);

        GpuCore::Instance().GetCommand()->Finish(context);
    };
};
} // namespace graphics
#endif