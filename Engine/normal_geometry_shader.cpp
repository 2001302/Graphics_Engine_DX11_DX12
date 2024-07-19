#include "normal_geometry_shader.h"
#include "geometry_generator.h"
#include "pipeline_manager.h"
#include "setting_ui.h"

using namespace dx11;

void NormalGeometryShaderSource::InitializeThis() {
    GraphicsContext::Instance().CreateConstantBuffer(
        vertex_constant, vertex_constant_buffer);
    GraphicsContext::Instance().CreateConstantBuffer(pixel_constant,
                                                     pixel_constant_buffer);
}

EnumBehaviorTreeStatus InitializeNormalGeometryShader::OnInvoke() {
    auto block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<dx11::PipelineManager *>(block);
    assert(manager != nullptr);

    // Create and initialize the light shader object.
    auto geometry_shader = std::make_shared<NormalGeometryShader>();
    manager->shaders[EnumShaderType::eNormalGeometry] = geometry_shader;

    // Texture sampler 만들기
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // Create the Sample State
    GraphicsContext::Instance().device()->CreateSamplerState(
        &sampDesc, geometry_shader->sample_state.GetAddressOf());

    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    GraphicsContext::Instance().CreateVertexShaderAndInputLayout(
        L"physically_vertex_shader.hlsl", inputElements,
        geometry_shader->vertex_shader, geometry_shader->layout);

    // Geometry shader 초기화하기
    GraphicsContext::Instance().CreateGeometryShader(
        L"NormalGS.hlsl", geometry_shader->normalGeometryShader);

    GraphicsContext::Instance().CreateVertexShaderAndInputLayout(
        L"NormalVS.hlsl", inputElements, geometry_shader->vertex_shader,
        geometry_shader->layout);

    GraphicsContext::Instance().CreatePixelShader(
        L"NormalPS.hlsl", geometry_shader->pixel_shader);

    return EnumBehaviorTreeStatus::eSuccess;
}
