#ifndef _TONEMAPPING_PSO
#define _TONEMAPPING_PSO

#include "graphics_pso.h"
#include "graphics_util.h"

namespace dx12 {
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
        HRESULT hr = D3D12SerializeVersionedRootSignature(&rootSignatureDesc,
                                                          &signature, &error);

        hr = dx12::GpuCore::Instance().device->CreateRootSignature(
            0, signature->GetBufferPointer(), signature->GetBufferSize(),
            IID_PPV_ARGS(&root_signature));

        // shader
        ComPtr<ID3DBlob> tone_mappingVS;
        dx12::Util::CreateVertexShader(dx12::GpuCore::Instance().device,
                                       L"graphics/ToneMappingVS.hlsl",
                                       tone_mappingVS);
        ComPtr<ID3DBlob> tone_mappingPS;
        dx12::Util::CreatePixelShader(dx12::GpuCore::Instance().device,
                                      L"graphics/ToneMappingPS.hlsl",
                                      tone_mappingPS);
        // pipeline state
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {dx12::layout::combineIEs,
                               _countof(dx12::layout::combineIEs)};
        psoDesc.pRootSignature = root_signature;
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(tone_mappingVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(tone_mappingPS.Get());
        psoDesc.RasterizerState =
            CD3DX12_RASTERIZER_DESC(dx12::rasterizer::postRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.StencilEnable = false;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleDesc.Quality = 0;

        dx12::ThrowIfFailed(
            dx12::GpuCore::Instance().device->CreateGraphicsPipelineState(
                &psoDesc, IID_PPV_ARGS(&pipeline_state)));
    };
    void Render(ComPtr<ID3D12GraphicsCommandList> command_list,
                ComPtr<ID3D12DescriptorHeap> sampler,
                ComPtr<ID3D12Resource> const_buffer,
                D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view,
                D3D12_INDEX_BUFFER_VIEW index_buffer_view, UINT index_count) {

        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            dx12::GpuCore::Instance()
                .resource_FLIP[dx12::GpuCore::Instance().frame_index]
                .Get(),
            D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
        command_list->ResourceBarrier(1, &barrier);

        barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            dx12::GpuCore::Instance().resource_LDR.Get(),
            D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        command_list->ResourceBarrier(1, &barrier);

        auto handle_resolved =
            dx12::GpuCore::Instance()
                .heap_LDR->GetGPUDescriptorHandleForHeapStart();
        auto handle_back_buffer = dx12::GpuCore::Instance().GetHandleFLIP();

        ID3D12DescriptorHeap *descriptorHeaps[] = {
            dx12::GpuCore::Instance().heap_LDR.Get(), sampler.Get()};

        // Set the compute root signature and pipeline state
        command_list->RSSetViewports(1, &dx12::GpuCore::Instance().viewport);
        command_list->RSSetScissorRects(1,
                                        &dx12::GpuCore::Instance().scissorRect);
        command_list->OMSetRenderTargets(1, &handle_back_buffer, false,
                                         nullptr);
        command_list->SetGraphicsRootSignature(root_signature);
        command_list->SetPipelineState(pipeline_state);

        command_list->SetDescriptorHeaps(_countof(descriptorHeaps),
                                         descriptorHeaps);
        // 1:constant buffer
        command_list->SetGraphicsRootConstantBufferView(
            0, const_buffer->GetGPUVirtualAddress());
        // 2:sampler
        command_list->SetGraphicsRootDescriptorTable(
            1, sampler->GetGPUDescriptorHandleForHeapStart());
        // 3:srv
        command_list->SetGraphicsRootDescriptorTable(2, handle_resolved);

        command_list->IASetPrimitiveTopology(
            D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        command_list->IASetVertexBuffers(0, 1, &vertex_buffer_view);
        command_list->IASetIndexBuffer(&index_buffer_view);
        command_list->DrawIndexedInstanced(index_count, 1, 0, 0, 0);

        barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            dx12::GpuCore::Instance()
                .resource_FLIP[dx12::GpuCore::Instance().frame_index]
                .Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
        command_list->ResourceBarrier(1, &barrier);

        barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            dx12::GpuCore::Instance().resource_LDR.Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_COMMON);
        command_list->ResourceBarrier(1, &barrier);
    };
};
} // namespace dx12
#endif