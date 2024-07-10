#include "ground_shader.h"
#include "geometry_generator.h"
#include "panel.h"
#include "resource_helper.h"

using namespace Engine;

void GroundShaderSource::InitializeThis() {
    vertex_constant_buffer_data.model = Matrix();
    vertex_constant_buffer_data.view = Matrix();
    vertex_constant_buffer_data.projection = Matrix();
    vertex_constant_buffer_data.useHeightMap = 0;
    vertex_constant_buffer_data.heightScale = 0.0f;

    Direct3D::GetInstance().CreateConstantBuffer(vertex_constant_buffer_data,
                                                 vertex_constant_buffer);
    Direct3D::GetInstance().CreateConstantBuffer(pixel_constant_buffer_data,
                                                 pixel_constant_buffer);
}

EnumBehaviorTreeStatus InitializeGroundShader::OnInvoke() {
    IDataBlock *block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<Engine::PipelineManager *>(block);
    assert(manager != nullptr);

    manager->ground = std::make_shared<Ground>();
    GeometryGenerator::MakeSquareGrid(manager->ground.get(), 256, 256, 20.0f,
                                      {40.0f, 40.0f});

    auto graph = std::make_shared<Graph>();
    graph->SetDetailNode(std::make_shared<ModelDetailNode>());
    manager->ground->behavior = graph;

    auto ground_shader = std::make_shared<GroundShader>();
    manager->shaders[EnumShaderType::eGround] = ground_shader;

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
    Direct3D::GetInstance().device()->CreateSamplerState(
        &sampDesc, ground_shader->sample_state.GetAddressOf());

    manager->ground->ground_shader_source =
        std::make_shared<GroundShaderSource>();
    auto ground_shader_source = manager->ground->ground_shader_source;

    ground_shader_source->vertex_constant_buffer_data.model = Matrix();
    ground_shader_source->vertex_constant_buffer_data.view = Matrix();
    ground_shader_source->vertex_constant_buffer_data.projection = Matrix();

    Direct3D::GetInstance().CreateConstantBuffer(
        ground_shader_source->vertex_constant_buffer_data,
        ground_shader_source->vertex_constant_buffer);
    Direct3D::GetInstance().CreateConstantBuffer(
        ground_shader_source->pixel_constant_buffer_data,
        ground_shader_source->pixel_constant_buffer);

    std::vector<D3D11_INPUT_ELEMENT_DESC> basicInputElements = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    Direct3D::GetInstance().CreateVertexShaderAndInputLayout(
        L"ground_vs.hlsl", basicInputElements, ground_shader->vertex_shader,
        ground_shader->layout);

    Direct3D::GetInstance().CreatePixelShader(L"ground_ps.hlsl",
                                              ground_shader->pixel_shader);

    auto mesh = manager->ground->mesh;
    Direct3D::GetInstance().CreateVertexBuffer(mesh->vertices,
                                               mesh->vertexBuffer);
    Direct3D::GetInstance().CreateIndexBuffer(mesh->indices, mesh->indexBuffer);

    mesh->albedoTextureFilename =
        "Assets/Textures/PBR/Bricks075A_1K-PNG/Bricks075A_1K_Color.png";

    mesh->normalTextureFilename = "Assets/Textures/PBR/Bricks075A_1K-PNG/"
                                  "Bricks075A_1K_NormalDX.png";

    mesh->heightTextureFilename = "Assets/Textures/PBR/Bricks075A_1K-PNG/"
                                  "Bricks075A_1K_Displacement.png";

    mesh->aoTextureFilename = "Assets/Textures/PBR/Bricks075A_1K-PNG/"
                              "Bricks075A_1K_AmbientOcclusion.png";

    if (!mesh->albedoTextureFilename.empty()) {

        std::cout << mesh->albedoTextureFilename << std::endl;
        ResourceHelper::CreateTexture(mesh->albedoTextureFilename,
                                      mesh->albedoTexture, mesh->albedoSRV);
    }

    if (!mesh->normalTextureFilename.empty()) {
        std::cout << mesh->normalTextureFilename << std::endl;
        ResourceHelper::CreateTexture(mesh->normalTextureFilename,
                                      mesh->normalTexture, mesh->normalSRV);
    }

    if (!mesh->heightTextureFilename.empty()) {
        std::cout << mesh->heightTextureFilename << std::endl;
        ResourceHelper::CreateTexture(mesh->heightTextureFilename,
                                      mesh->heightTexture, mesh->heightSRV);
    }

    if (!mesh->aoTextureFilename.empty()) {
        std::cout << mesh->aoTextureFilename << std::endl;
        ResourceHelper::CreateTexture(mesh->aoTextureFilename, mesh->aoTexture,
                                      mesh->aoSRV);
    }

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus UpdateGroundShader::OnInvoke() {
    IDataBlock *managerBlock = data_block[EnumDataBlockType::eManager];
    IDataBlock *guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<Engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto gui = dynamic_cast<Engine::Panel *>(guiBlock);
    assert(gui != nullptr);

    auto ground = manager->ground;

    auto ground_shader_source = ground->ground_shader_source;
    auto ground_shader = manager->shaders[EnumShaderType::eGround];

    {
        ground_shader_source->vertex_constant_buffer_data.model =
            Matrix::CreateRotationX(DirectX::XM_PIDIV2).Transpose();

        ground_shader_source->vertex_constant_buffer_data.invTranspose =
            ground_shader_source->vertex_constant_buffer_data.model;
        ground_shader_source->vertex_constant_buffer_data.invTranspose
            .Translation(Vector3(0.0f));
        ground_shader_source->vertex_constant_buffer_data.invTranspose =
            ground_shader_source->vertex_constant_buffer_data.invTranspose
                .Transpose()
                .Invert();

        ground_shader_source->vertex_constant_buffer_data.invTranspose =
            ground_shader_source->vertex_constant_buffer_data.model.Transpose();

        ground_shader_source->vertex_constant_buffer_data.view =
            manager->camera->view.Transpose();

        const float aspect = Env::Get().aspect;
        ground_shader_source->vertex_constant_buffer_data.projection =
            XMMatrixPerspectiveFovLH(70.0f, aspect, 0.01f, 100.0f);

        ground_shader_source->vertex_constant_buffer_data.projection =
            ground_shader_source->vertex_constant_buffer_data.projection
                .Transpose();
    }

    ground_shader_source->pixel_constant_buffer_data.useTexture = true;
    ground_shader_source->vertex_constant_buffer_data.useHeightMap = true;
    ground_shader_source->pixel_constant_buffer_data.useAOMap = true;
    ground_shader_source->pixel_constant_buffer_data.material.diffuse =
        Vector3(1.0f);
    ground_shader_source->pixel_constant_buffer_data.material.specular =
        Vector3(0.0f);
    ground_shader_source->pixel_constant_buffer_data.material.shininess = 0.1f;

    Direct3D::GetInstance().UpdateBuffer(
        ground_shader_source->vertex_constant_buffer_data,
        ground_shader_source->vertex_constant_buffer);

    Direct3D::GetInstance().UpdateBuffer(
        ground_shader_source->pixel_constant_buffer_data,
        ground_shader_source->pixel_constant_buffer);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus RenderGroundShader::OnInvoke() {
    IDataBlock *managerBlock = data_block[EnumDataBlockType::eManager];
    IDataBlock *guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<Engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto gui = dynamic_cast<Engine::Panel *>(guiBlock);
    assert(gui != nullptr);

    auto context = Direct3D::GetInstance().device_context();
    auto ground = manager->ground;
    auto ground_shader = manager->shaders[EnumShaderType::eGround];

    unsigned int stride = sizeof(Vertex);
    unsigned int offset = 0;

    // RS: Rasterizer stage
    // OM: Output-Merger stage
    // VS: Vertex Shader
    // PS: Pixel Shader
    // IA: Input-Assembler stage
    context->IASetInputLayout(ground_shader->layout.Get());
    context->IASetVertexBuffers(0, 1, ground->mesh->vertexBuffer.GetAddressOf(),
                                &stride, &offset);
    context->IASetIndexBuffer(ground->mesh->indexBuffer.Get(),
                              DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->VSSetShader(ground_shader->vertex_shader.Get(), 0, 0);
    context->VSSetShaderResources(0, 1, ground->mesh->heightSRV.GetAddressOf());
    context->VSSetSamplers(0, 1, ground_shader->sample_state.GetAddressOf());
    context->VSSetConstantBuffers(
        0, 1,
        ground->ground_shader_source->vertex_constant_buffer.GetAddressOf());

    if (gui->GetGlobalTab().draw_as_wire_)
        context->RSSetState(
            Direct3D::GetInstance().wire_rasterizer_state().Get());
    else
        context->RSSetState(
            Direct3D::GetInstance().solid_rasterizer_state().Get());

    context->PSSetSamplers(0, 1, ground_shader->sample_state.GetAddressOf());
    context->PSSetShader(ground_shader->pixel_shader.Get(), 0, 0);
    std::vector<ID3D11ShaderResourceView *> resViews = {
        nullptr, nullptr, ground->mesh->albedoSRV.Get(),
        ground->mesh->normalSRV.Get(), ground->mesh->aoSRV.Get()};
    context->PSSetShaderResources(0, UINT(resViews.size()), resViews.data());

    context->PSSetConstantBuffers(
        0, 1,
        ground->ground_shader_source->pixel_constant_buffer.GetAddressOf());

    context->DrawIndexed(ground->GetIndexCount(), 0, 0);

    return EnumBehaviorTreeStatus::eSuccess;
}
