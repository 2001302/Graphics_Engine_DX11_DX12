#ifndef _MESH_PSO
#define _MESH_PSO

#include "graphics_pso.h"
#include "graphics_util.h"

namespace graphics {
class SolidMeshPSO : public GraphicsPSO {
  public:
    void Initialize() override {
        // rootSignature
        // s0 ~ s6
        CD3DX12_DESCRIPTOR_RANGE1 samplerRange;
        samplerRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 7, 0);

        // t10 ~ t16
        CD3DX12_DESCRIPTOR_RANGE1 textureRange;
        textureRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 10);

        CD3DX12_ROOT_PARAMETER1 rootParameters[7] = {};
        // Common.hlsli : s0~s6,t10~t16,b0~b2
        rootParameters[0].InitAsDescriptorTable(1, &samplerRange,
                                                D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[1].InitAsDescriptorTable(1, &textureRange,
                                                D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[2].InitAsConstantBufferView(
            0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[3].InitAsConstantBufferView(
            1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[4].InitAsConstantBufferView(
            2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            D3D12_SHADER_VISIBILITY_ALL);
        // VS : t0
        CD3DX12_DESCRIPTOR_RANGE1 textureRangeVS;
        textureRangeVS.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        rootParameters[5].InitAsDescriptorTable(1, &textureRangeVS,
                                                D3D12_SHADER_VISIBILITY_VERTEX);
        // PS : t0 ~ t4
        CD3DX12_DESCRIPTOR_RANGE1 textureRangePS;
        textureRangePS.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0);
        rootParameters[6].InitAsDescriptorTable(1, &textureRangePS,
                                                D3D12_SHADER_VISIBILITY_PIXEL);

        auto rootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(
            ARRAYSIZE(rootParameters), rootParameters, 0, nullptr,
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
        Util::CreateVertexShader(GpuCore::Instance().GetDevice(),
                                 L"Graphics/Shader/BasicVS.hlsl", basicVS);
        Util::CreatePixelShader(GpuCore::Instance().GetDevice(),
                                L"Graphics/Shader/BasicPS.hlsl", basicPS);
        //
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {layout::basicIEs, _countof(layout::basicIEs)};
        psoDesc.pRootSignature = root_signature;
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(basicVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(rasterizer::solidRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState = depth::basicDS;
        psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
        psoDesc.SampleDesc.Count = 4;
        psoDesc.SampleDesc.Quality = 0;

        ASSERT_FAILED(
            GpuCore::Instance().GetDevice()->CreateGraphicsPipelineState(
                &psoDesc, IID_PPV_ARGS(&pipeline_state)));
    };
    void Render(ComPtr<ID3D12GraphicsCommandList> command_list,
                CD3DX12_CPU_DESCRIPTOR_HANDLE render_target_view,
                CD3DX12_CPU_DESCRIPTOR_HANDLE depth_stencil_view,
                GpuResourceList *shared_texture, GpuResourceList *buffer_PS,
                GpuResourceList *buffer_VS, DynamicDescriptorHeap *gpu_heap,
                DynamicDescriptorHeap *sampler_heap,
                ComPtr<ID3D12Resource> global_consts,
                ComPtr<ID3D12Resource> mesh_consts,
                ComPtr<ID3D12Resource> material_consts,
                D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view,
                D3D12_INDEX_BUFFER_VIEW index_buffer_view, UINT index_count) {

        //auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        //    GpuCore::Instance().Buffer().hdr_buffer,
        //    D3D12_RESOURCE_STATE_COMMON,
        //    D3D12_RESOURCE_STATE_RENDER_TARGET);

        //command_list->ResourceBarrier(1, &barrier);

        ////command_list->RSSetViewports(1, &GpuDevice::Get().viewport);
        ////command_list->RSSetScissorRects(1, &GpuDevice::Get().scissorRect);
        //command_list->OMSetRenderTargets(1, &render_target_view, false,
        //                                 &depth_stencil_view);
        //command_list->SetGraphicsRootSignature(root_signature);
        //command_list->SetPipelineState(pipeline_state);

        //command_list->SetGraphicsRootDescriptorTable(
        //    0, sampler_heap->GetGpuHandle(0));
        //// global texture
        //command_list->SetGraphicsRootDescriptorTable(
        //    1, shared_texture->GetGpuHandle());
        //command_list->SetGraphicsRootConstantBufferView(
        //    2, global_consts->GetGPUVirtualAddress());
        //command_list->SetGraphicsRootConstantBufferView(
        //    3, mesh_consts.Get()->GetGPUVirtualAddress());
        //command_list->SetGraphicsRootConstantBufferView(
        //    4, material_consts.Get()->GetGPUVirtualAddress());
        //command_list->SetGraphicsRootDescriptorTable(5,
        //                                             buffer_VS->GetGpuHandle());
        //command_list->SetGraphicsRootDescriptorTable(6,
        //                                             buffer_PS->GetGpuHandle());

        //command_list->IASetPrimitiveTopology(
        //    D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        //command_list->IASetVertexBuffers(0, 1, &vertex_buffer_view);
        //command_list->IASetIndexBuffer(&index_buffer_view);
        //command_list->DrawIndexedInstanced(index_count, 1, 0, 0, 0);

        //barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        //    GpuDevice::Get().resource_HDR.Get(),
        //    D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
        //command_list->ResourceBarrier(1, &barrier);
    };
};
class WireMeshPSO : public GraphicsPSO {
  public:
    void Initialize(){};
    void Render(){};
};
} // namespace graphics
#endif