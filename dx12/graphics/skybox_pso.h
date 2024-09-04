#ifndef _SKYBOX_PSO
#define _SKYBOX_PSO

#include "graphics_pso.h"
#include "graphics_util.h"

namespace graphics {
class SkyboxPSO : public GraphicsPSO {
  public:
    void Initialize() override {
        // rootSignature
        // s0 ~ s6
        CD3DX12_DESCRIPTOR_RANGE1 samplerRange;
        samplerRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

        // t10 ~ t16
        CD3DX12_DESCRIPTOR_RANGE1 textureRange;
        textureRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 10);

        // rootSignature
        CD3DX12_ROOT_PARAMETER1 rootParameters[5] = {};
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

        auto rootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(
            ARRAYSIZE(rootParameters), rootParameters, 0, nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        HRESULT hr = D3D12SerializeVersionedRootSignature(&rootSignatureDesc,
                                                          &signature, &error);

        hr = GpuCore::Instance().device->CreateRootSignature(
            0, signature->GetBufferPointer(), signature->GetBufferSize(),
            IID_PPV_ARGS(&root_signature));

        // shader
        ComPtr<ID3DBlob> skyboxVS;
        ComPtr<ID3DBlob> skyboxPS;
        Util::CreateVertexShader(GpuCore::Instance().device,
                                 L"Graphics/Shader/SkyboxVS.hlsl", skyboxVS);
        Util::CreatePixelShader(GpuCore::Instance().device,
                                L"Graphics/Shader/SkyboxPS.hlsl", skyboxPS);
        // pipeline state
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {layout::basicIEs, _countof(layout::basicIEs)};
        psoDesc.pRootSignature = root_signature;
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(skyboxVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(skyboxPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(rasterizer::solidRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.StencilEnable = false;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
        psoDesc.SampleDesc.Count = 4;
        psoDesc.SampleDesc.Quality = 0;

        ThrowIfFailed(GpuCore::Instance().device->CreateGraphicsPipelineState(
            &psoDesc, IID_PPV_ARGS(&pipeline_state)));
    };
    void Render(ComPtr<ID3D12GraphicsCommandList> command_list,
                CD3DX12_CPU_DESCRIPTOR_HANDLE render_target_view,
                CD3DX12_CPU_DESCRIPTOR_HANDLE depth_stencil_view,
                GpuBufferList *shared_texture, GpuHeap *gpu_heap,
                ComPtr<ID3D12Resource> global_consts,
                ComPtr<ID3D12Resource> mesh_consts,
                ComPtr<ID3D12Resource> material_consts,
                D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view,
                D3D12_INDEX_BUFFER_VIEW index_buffer_view, UINT index_count) {

        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            GpuCore::Instance().resource_HDR.Get(), D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_RENDER_TARGET);
        command_list->ResourceBarrier(1, &barrier);

        command_list->RSSetViewports(1, &GpuCore::Instance().viewport);
        command_list->RSSetScissorRects(1, &GpuCore::Instance().scissorRect);
        command_list->OMSetRenderTargets(1, &render_target_view, false,
                                         &depth_stencil_view);
        command_list->SetGraphicsRootSignature(root_signature);
        command_list->SetPipelineState(pipeline_state);

        command_list->SetGraphicsRootDescriptorTable(
            0, gpu_heap->GetSamplerGpuHandle());
        command_list->SetGraphicsRootDescriptorTable(
            1, shared_texture->GetGpuHandle());
        // global texture
        command_list->SetGraphicsRootConstantBufferView(
            2, global_consts->GetGPUVirtualAddress());
        command_list->SetGraphicsRootConstantBufferView(
            3, mesh_consts.Get()->GetGPUVirtualAddress());
        command_list->SetGraphicsRootConstantBufferView(
            4, material_consts.Get()->GetGPUVirtualAddress());

        command_list->IASetPrimitiveTopology(
            D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        command_list->IASetVertexBuffers(0, 1, &vertex_buffer_view);
        command_list->IASetIndexBuffer(&index_buffer_view);
        command_list->DrawIndexedInstanced(index_count, 1, 0, 0, 0);

        barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            GpuCore::Instance().resource_HDR.Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
        command_list->ResourceBarrier(1, &barrier);
    };
};
} // namespace core
#endif