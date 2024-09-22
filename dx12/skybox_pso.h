#ifndef _SKYBOX_PSO
#define _SKYBOX_PSO

#include "pipeline_state_object.h"
#include "shader_util.h"

namespace graphics {
class SkyboxPSO : public GraphicsPSO {
  public:
    void Initialize() override {
        // rootSignature
        // s0 ~ s6
        CD3DX12_DESCRIPTOR_RANGE1 sampler_range;
        sampler_range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

        // t10 ~ t16
        CD3DX12_DESCRIPTOR_RANGE1 texture_range;
        texture_range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 10);

        // rootSignature
        CD3DX12_ROOT_PARAMETER1 root_parameters[5] = {};
        // Common.hlsli : s0~s6,t10~t16,b0~b2
        root_parameters[0].InitAsDescriptorTable(1, &sampler_range,
                                                 D3D12_SHADER_VISIBILITY_ALL);
        root_parameters[1].InitAsDescriptorTable(1, &texture_range,
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

        auto root_signature_desc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(
            ARRAYSIZE(root_parameters), root_parameters, 0, nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ASSERT_FAILED(D3D12SerializeVersionedRootSignature(&root_signature_desc,
                                                           &signature, &error));

        ASSERT_FAILED(GpuCore::Instance().GetDevice()->CreateRootSignature(
            0, signature->GetBufferPointer(), signature->GetBufferSize(),
            IID_PPV_ARGS(&root_signature)));

        // shader
        ComPtr<ID3DBlob> skyboxVS;
        ComPtr<ID3DBlob> skyboxPS;
        ShaderUtil::CreateVertexShader(GpuCore::Instance().GetDevice(),
                                       L"Shader/SkyboxVS.hlsl", skyboxVS);
        ShaderUtil::CreatePixelShader(GpuCore::Instance().GetDevice(),
                                      L"Shader/SkyboxPS.hlsl", skyboxPS);
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
        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleDesc.Quality = 0;

        ASSERT_FAILED(
            GpuCore::Instance().GetDevice()->CreateGraphicsPipelineState(
                &psoDesc, IID_PPV_ARGS(&pipeline_state)));
    };
    void Render(SamplerState *shared_sampler, GpuResourceList *shared_texture,
                ComPtr<ID3D12Resource> global_consts,
                MeshRenderer *mesh_renderer) {

        auto context =
            GpuCore::Instance().GetCommand()->Begin<GraphicsCommandContext>(
                L"Skybox");

        context->SetDescriptorHeaps(
            std::vector{GpuCore::Instance().GetHeap().View(),
                        GpuCore::Instance().GetHeap().Sampler()});

        context->TransitionResource(GpuBuffer::Instance().GetHDR(),
                                    D3D12_RESOURCE_STATE_RENDER_TARGET, true);

        context->SetViewportAndScissorRect(0, 0,
                                           (UINT)common::env::screen_width,
                                           (UINT)common::env::screen_height);

        context->SetRenderTargetView(GpuBuffer::Instance().GetHDR());

        context->SetRootSignature(root_signature);
        context->SetPipelineState(pipeline_state);

        context->SetGraphicsRootDescriptorTable(0,
                                                shared_sampler->GetGpuHandle());
        context->SetGraphicsRootDescriptorTable(1,
                                                shared_texture->GetGpuHandle());
        // global texture
        context->SetGraphicsRootConstantBufferView(
            2, global_consts->GetGPUVirtualAddress());
        context->SetGraphicsRootConstantBufferView(
            3, mesh_renderer->GetMeshConsts()->GetGPUVirtualAddress());
        context->SetGraphicsRootConstantBufferView(
            4, mesh_renderer->GetMaterialConsts()->GetGPUVirtualAddress());

        context->SetTrimitiveTopology(
            D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->SetVertexBuffers(
            &mesh_renderer->GetMeshes().front()->vertex_buffer_view);
        context->SetIndexBuffers(
            &mesh_renderer->GetMeshes().front()->index_buffer_view);
        context->DrawIndexedInstanced(
            mesh_renderer->GetMeshes().front()->index_count);

        GpuCore::Instance().GetCommand()->Finish(context);
    };
};
} // namespace graphics
#endif