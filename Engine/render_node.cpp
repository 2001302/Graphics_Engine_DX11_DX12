#include "render_node.h"
#include "game_object_node.h"
#include "geometry_generator.h"
#include "panel.h"
#include "pipeline_manager.h"

using namespace Engine;

EnumBehaviorTreeStatus InitializeCamera::OnInvoke() {
    IDataBlock *block = DataBlock[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<Engine::PipelineManager *>(block);
    assert(manager != nullptr);

    // Create and initialize the camera object.
    manager->camera = std::make_unique<Camera>();

    manager->camera->position =
        DirectX::SimpleMath::Vector3(0.0f, 0.0f, -10.0f);
    manager->camera->Render();

    // Update the position and rotation of the camera for this scene.
    manager->camera->position =
        DirectX::SimpleMath::Vector3(-10.0f, 0.0f, -10.0f);
    manager->camera->rotation = DirectX::SimpleMath::Vector3(0.0f, 45.0f, 0.0f);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus InitializePhongShader::OnInvoke() {
    IDataBlock *block = DataBlock[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<Engine::PipelineManager *>(block);
    assert(manager != nullptr);

    // Create and initialize the light shader object.
    auto phong_shader = std::make_shared<PhongShader>();
    manager->shaders[EnumShaderType::ePhong] = phong_shader;

    // Texture sampler �����
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
    Direct3D::GetInstance().GetDevice()->CreateSamplerState(
        &sampDesc, phong_shader->sample_state.GetAddressOf());

    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 3,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    phong_shader->CreateVertexShaderAndInputLayout(
        L"phong_vertex_shader.hlsl", inputElements, phong_shader->vertex_shader,
        phong_shader->layout);

    phong_shader->CreatePixelShader(L"phong_pixel_shader.hlsl",
                                    phong_shader->pixel_shader);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus UpdateCamera::OnInvoke() {
    IDataBlock *managerBlock = DataBlock[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<Engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    // Generate the view matrix based on the camera's position.
    manager->camera->Render();

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus UpdateGameObjects::OnInvoke() {
    IDataBlock *managerBlock = DataBlock[EnumDataBlockType::eManager];
    IDataBlock *envBlock = DataBlock[EnumDataBlockType::eEnv];
    IDataBlock *guiBlock = DataBlock[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<Engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto env = dynamic_cast<Engine::Env *>(envBlock);
    assert(env != nullptr);

    auto gui = dynamic_cast<Engine::Panel *>(guiBlock);
    assert(gui != nullptr);

    auto context = Direct3D::GetInstance().GetDeviceContext();

    for (auto &model_amp : manager->models) {
        auto &model = model_amp.second;
        auto graph = manager->behaviors[model->GetEntityId()];

        auto detail = dynamic_cast<Engine::GameObjectDetailNode *>(
            graph->GetDetailNode().get());
        assert(detail != nullptr);

        auto phong_shader_source = model->phong_shader_source;
        auto phong_shader = manager->shaders[EnumShaderType::ePhong];
        // model
        {
            phong_shader_source->vertex_constant_buffer_data.model =
                Matrix::CreateScale(detail->scaling) *
                Matrix::CreateRotationX(detail->rotation.x) *
                Matrix::CreateRotationY(detail->rotation.y) *
                Matrix::CreateRotationZ(detail->rotation.z) *
                Matrix::CreateTranslation(detail->translation);

            phong_shader_source->vertex_constant_buffer_data.model =
                phong_shader_source->vertex_constant_buffer_data.model
                    .Transpose();
        }
        // view
        {
            phong_shader_source->vertex_constant_buffer_data.view =
                manager->camera->view.Transpose();
        }
        // inverse transpose
        {
            phong_shader_source->vertex_constant_buffer_data.invTranspose =
                phong_shader_source->vertex_constant_buffer_data.model;
            phong_shader_source->vertex_constant_buffer_data.invTranspose
                .Translation(Vector3(0.0f));
            phong_shader_source->vertex_constant_buffer_data.invTranspose =
                phong_shader_source->vertex_constant_buffer_data.invTranspose
                    .Transpose()
                    .Invert();
        }
        // projection
        {
            const float aspect = env->aspect_;
            if (detail->use_perspective_projection) {
                phong_shader_source->vertex_constant_buffer_data.projection =
                    XMMatrixPerspectiveFovLH(
                        XMConvertToRadians(
                            gui->GetLightTab().projection_fov_angle_y),
                        aspect, gui->GetLightTab().near_z,
                        gui->GetLightTab().far_z);
            } else {
                phong_shader_source->vertex_constant_buffer_data.projection =
                    XMMatrixOrthographicOffCenterLH(
                        -aspect, aspect, -1.0f, 1.0f, gui->GetLightTab().near_z,
                        gui->GetLightTab().far_z);
            }
            phong_shader_source->vertex_constant_buffer_data.projection =
                phong_shader_source->vertex_constant_buffer_data.projection
                    .Transpose();
        }

        phong_shader->UpdateBuffer(
            phong_shader_source->vertex_constant_buffer_data,
            phong_shader_source->vertex_constant_buffer);

        // eye
        {
            phong_shader_source->pixel_constant_buffer_data
                .eyeWorld = Vector3::Transform(
                Vector3(0.0f),
                phong_shader_source->vertex_constant_buffer_data.view.Invert());
        }
        // material
        {
            phong_shader_source->pixel_constant_buffer_data.material.diffuse =
                Vector3(detail->diffuse);
            phong_shader_source->pixel_constant_buffer_data.material.specular =
                Vector3(detail->specular);
            phong_shader_source->pixel_constant_buffer_data.material.shininess =
                detail->shininess;
        }
        // light
        {
            for (int i = 0; i < MAX_LIGHTS; i++) {
                if (i != gui->GetLightTab().light_type) {
                    phong_shader_source->pixel_constant_buffer_data.lights[i]
                        .strength *= 0.0f;
                } else {
                    // turn off another light
                    phong_shader_source->pixel_constant_buffer_data.lights[i] =
                        gui->GetLightTab().light_from_gui;
                }
            }
        }

        phong_shader_source->pixel_constant_buffer_data.useTexture =
            detail->use_texture;
        phong_shader_source->pixel_constant_buffer_data.useBlinnPhong =
            detail->use_blinn_phong;

        phong_shader->UpdateBuffer(
            phong_shader_source->pixel_constant_buffer_data,
            phong_shader_source->pixel_constant_buffer);
    }
    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus RenderGameObjects::OnInvoke() {
    IDataBlock *managerBlock = DataBlock[EnumDataBlockType::eManager];
    IDataBlock *envBlock = DataBlock[EnumDataBlockType::eEnv];
    IDataBlock *guiBlock = DataBlock[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<Engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto env = dynamic_cast<Engine::Env *>(envBlock);
    assert(env != nullptr);

    auto gui = dynamic_cast<Engine::Panel *>(guiBlock);
    assert(gui != nullptr);

    auto context = Direct3D::GetInstance().GetDeviceContext();

    float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    context->ClearRenderTargetView(Direct3D::GetInstance().render_target_view_,
                                   clearColor);
    context->ClearDepthStencilView(
        Direct3D::GetInstance().depth_stencil_view_.Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    for (auto &model_map : manager->models) {
        // RS: Rasterizer stage
        // OM: Output-Merger stage
        // VS: Vertex Shader
        // PS: Pixel Shader
        // IA: Input-Assembler stage
        auto model = model_map.second;
        auto phong_shader_source = model->phong_shader_source;
        auto phong_shader = manager->shaders[EnumShaderType::ePhong];

        unsigned int stride = sizeof(Vertex);
        unsigned int offset = 0;

        context->OMSetRenderTargets(
            1, &Direct3D::GetInstance().render_target_view_,
            Direct3D::GetInstance().depth_stencil_view_.Get());
        context->OMSetDepthStencilState(
            Direct3D::GetInstance().depth_stencil_state_.Get(), 0);

        context->VSSetShader(phong_shader->vertex_shader.Get(), 0, 0);
        context->PSSetSamplers(0, 1, &phong_shader->sample_state);

        if (gui->GetLightTab().draw_as_wire_)
            context->RSSetState(
                Direct3D::GetInstance().wire_rasterizer_state_.Get());
        else
            context->RSSetState(
                Direct3D::GetInstance().solid_rasterizer_state_.Get());

        context->VSSetConstantBuffers(
            0, 1, phong_shader_source->vertex_constant_buffer.GetAddressOf());

        for (const auto &mesh : model->meshes) {
            context->PSSetShaderResources(
                0, 1, mesh->textureResourceView.GetAddressOf());

            context->PSSetConstantBuffers(
                0, 1,
                phong_shader_source->pixel_constant_buffer.GetAddressOf());
            context->PSSetShader(phong_shader->pixel_shader.Get(), NULL, 0);

            context->IASetInputLayout(phong_shader->layout.Get());
            context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(),
                                        &stride, &offset);
            context->IASetIndexBuffer(mesh->indexBuffer.Get(),
                                      DXGI_FORMAT_R32_UINT, 0);
            context->IASetPrimitiveTopology(
                D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            context->DrawIndexed(model->GetIndexCount(), 0, 0);
        }
    }

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus InitializeCubeMapShader::OnInvoke() {
    IDataBlock *block = DataBlock[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<Engine::PipelineManager *>(block);
    assert(manager != nullptr);

    manager->cube_map = std::make_shared<CubeMap>();
    GeometryGenerator::MakeSphere(manager->cube_map.get(), 20.0f, 15, 13);

    auto graph = std::make_shared<Graph>();
    graph->SetDetailNode(std::make_shared<GameObjectDetailNode>());
    manager->behaviors[manager->cube_map->GetEntityId()] = graph;

    auto cube_map_shader = std::make_shared<CubeMapShader>();
    manager->shaders[EnumShaderType::eCube] = cube_map_shader;

    std::reverse(manager->cube_map->mesh->indices.begin(),
                 manager->cube_map->mesh->indices.end());

    auto skyboxFilename = L"./CubemapTextures/skybox.dds";
    auto nightPathFilename = L"./CubemapTextures/HumusTextures/NightPath.dds";
    auto atribumDiffuseFilename = L"./CubemapTextures/Atrium_diffuseIBL.dds";
    auto atribumSpecularFilename = L"./CubemapTextures/Atrium_specularIBL.dds";
    auto stonewallSpecularFilename =
        L"./CubemapTextures/Stonewall_specularIBL.dds";
    auto stonewallDiffuseFilename =
        L"./CubemapTextures/Stonewall_diffuseIBL.dds";

    cube_map_shader->CreateCubemapTexture(
        atribumDiffuseFilename, manager->cube_map->diffuse_resource_view);
    cube_map_shader->CreateCubemapTexture(
        skyboxFilename, manager->cube_map->specular_resource_view);

    manager->cube_map->cube_map_shader_source =
        std::make_shared<CubeMapShaderSource>();
    auto cube_map_shader_source = manager->cube_map->cube_map_shader_source;

    cube_map_shader_source->vertex_constant_buffer_data.model = Matrix();
    cube_map_shader_source->vertex_constant_buffer_data.view = Matrix();
    cube_map_shader_source->vertex_constant_buffer_data.projection = Matrix();

    cube_map_shader->CreateConstantBuffer(
        cube_map_shader_source->vertex_constant_buffer_data,
        cube_map_shader_source->vertex_constant_buffer);
    cube_map_shader->CreateConstantBuffer(
        cube_map_shader_source->pixel_constant_buffer_data,
        cube_map_shader_source->pixel_constant_buffer);

    cube_map_shader->CreateVertexBuffer(manager->cube_map->mesh->vertices,
                                        manager->cube_map->mesh->vertexBuffer);
    cube_map_shader->CreateIndexBuffer(manager->cube_map->mesh->indices,
                                       manager->cube_map->mesh->indexBuffer);

    std::vector<D3D11_INPUT_ELEMENT_DESC> basicInputElements = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 3,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    cube_map_shader->CreateVertexShaderAndInputLayout(
        L"cube_mapping_vertex_shader.hlsl", basicInputElements,
        cube_map_shader->vertex_shader, cube_map_shader->layout);

    cube_map_shader->CreatePixelShader(L"cube_mapping_pixel_shader.hlsl",
                                       cube_map_shader->pixel_shader);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus UpdateCubeMap::OnInvoke() {
    IDataBlock *managerBlock = DataBlock[EnumDataBlockType::eManager];
    IDataBlock *envBlock = DataBlock[EnumDataBlockType::eEnv];
    IDataBlock *guiBlock = DataBlock[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<Engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto env = dynamic_cast<Engine::Env *>(envBlock);
    assert(env != nullptr);

    auto cube_map = manager->cube_map;

    auto cube_shader_source = cube_map->cube_map_shader_source;
    auto cube_map_shader = manager->shaders[EnumShaderType::eCube];

    {
        cube_shader_source->vertex_constant_buffer_data.view =
            manager->camera->view.Transpose();

        const float aspect = env->aspect_;
        cube_shader_source->vertex_constant_buffer_data.projection =
            XMMatrixPerspectiveFovLH(70.0f, aspect, 0.01f, 100.0f);

        cube_shader_source->vertex_constant_buffer_data.projection =
            cube_shader_source->vertex_constant_buffer_data.projection
                .Transpose();
    }

    cube_map_shader->UpdateBuffer(
        cube_shader_source->vertex_constant_buffer_data,
        cube_shader_source->vertex_constant_buffer);

    cube_map_shader->UpdateBuffer(
        cube_shader_source->pixel_constant_buffer_data,
        cube_shader_source->pixel_constant_buffer);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus RenderCubeMap::OnInvoke() {
    IDataBlock *managerBlock = DataBlock[EnumDataBlockType::eManager];
    IDataBlock *envBlock = DataBlock[EnumDataBlockType::eEnv];
    IDataBlock *guiBlock = DataBlock[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<Engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto env = dynamic_cast<Engine::Env *>(envBlock);
    assert(env != nullptr);

    auto gui = dynamic_cast<Engine::Panel *>(guiBlock);
    assert(gui != nullptr);

    auto context = Direct3D::GetInstance().GetDeviceContext();
    auto cube_map = manager->cube_map;
    auto cube_map_shader = manager->shaders[EnumShaderType::eCube];

    unsigned int stride = sizeof(Vertex);
    unsigned int offset = 0;

    // RS: Rasterizer stage
    // OM: Output-Merger stage
    // VS: Vertex Shader
    // PS: Pixel Shader
    // IA: Input-Assembler stage
    context->IASetInputLayout(cube_map_shader->layout.Get());
    context->IASetVertexBuffers(
        0, 1, cube_map->mesh->vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(cube_map->mesh->indexBuffer.Get(),
                              DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->VSSetShader(cube_map_shader->vertex_shader.Get(), 0, 0);
    context->VSSetConstantBuffers(0, 1,
                                  cube_map->cube_map_shader_source
                                      ->vertex_constant_buffer.GetAddressOf());
    ID3D11ShaderResourceView *views[2] = {
        cube_map->diffuse_resource_view.Get(),
        cube_map->specular_resource_view.Get()};
    context->PSSetShaderResources(0, 2, views);
    context->PSSetShader(cube_map_shader->pixel_shader.Get(), 0, 0);
    context->PSSetSamplers(0, 1, cube_map_shader->sample_state.GetAddressOf());

    context->DrawIndexed(cube_map->GetIndexCount(), 0, 0);

    return EnumBehaviorTreeStatus::eSuccess;
}