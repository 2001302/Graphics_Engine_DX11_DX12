#include "render_node.h"
#include "game_object_node.h"
#include "geometry_generator.h"
#include "panel.h"
#include "pipeline_manager.h"

using namespace Engine;

EnumBehaviorTreeStatus InitializeCamera::OnInvoke() {
    IDataBlock *block = data_block[EnumDataBlockType::eManager];

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

EnumBehaviorTreeStatus UpdateCamera::OnInvoke() {
    IDataBlock *managerBlock = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<Engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    // Generate the view matrix based on the camera's position.
    manager->camera->Render();

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus InitializePhongShader::OnInvoke() {
    IDataBlock *block = data_block[EnumDataBlockType::eManager];

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
    Direct3D::GetInstance().device()->CreateSamplerState(
        &sampDesc, phong_shader->sample_state.GetAddressOf());

    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 3,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    Direct3D::GetInstance().CreateVertexShaderAndInputLayout(
        L"phong_vertex_shader.hlsl", inputElements, phong_shader->vertex_shader,
        phong_shader->layout);

    Direct3D::GetInstance().CreatePixelShader(L"phong_pixel_shader.hlsl",
                                              phong_shader->pixel_shader);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus UpdateGameObjectsUsingPhongShader::OnInvoke() {
    IDataBlock *managerBlock = data_block[EnumDataBlockType::eManager];
    IDataBlock *envBlock = data_block[EnumDataBlockType::eEnv];
    IDataBlock *guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<Engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto env = dynamic_cast<Engine::Env *>(envBlock);
    assert(env != nullptr);

    auto gui = dynamic_cast<Engine::Panel *>(guiBlock);
    assert(gui != nullptr);

    auto context = Direct3D::GetInstance().device_context();

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
                            gui->GetGlobalTab()
                                .projection_setting.projection_fov_angle_y),
                        aspect, gui->GetGlobalTab().projection_setting.near_z,
                        gui->GetGlobalTab().projection_setting.far_z);
            } else {
                phong_shader_source->vertex_constant_buffer_data.projection =
                    XMMatrixOrthographicOffCenterLH(
                        -aspect, aspect, -1.0f, 1.0f,
                        gui->GetGlobalTab().projection_setting.near_z,
                        gui->GetGlobalTab().projection_setting.far_z);
            }
            phong_shader_source->vertex_constant_buffer_data.projection =
                phong_shader_source->vertex_constant_buffer_data.projection
                    .Transpose();
        }

        Direct3D::GetInstance().UpdateBuffer(
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
                if (i != gui->GetGlobalTab().light_setting.light_type) {
                    phong_shader_source->pixel_constant_buffer_data.lights[i]
                        .strength *= 0.0f;
                } else {
                    // turn off another light
                    phong_shader_source->pixel_constant_buffer_data.lights[i] =
                        gui->GetGlobalTab().light_setting.light_from_gui;
                }
            }
        }

        phong_shader_source->pixel_constant_buffer_data.useTexture =
            detail->use_texture;
        phong_shader_source->pixel_constant_buffer_data.useBlinnPhong =
            gui->GetGlobalTab().light_setting.use_blinn_phong;

        Direct3D::GetInstance().UpdateBuffer(
            phong_shader_source->pixel_constant_buffer_data,
            phong_shader_source->pixel_constant_buffer);
    }
    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus RenderGameObjectsUsingPhongShader::OnInvoke() {
    IDataBlock *managerBlock = data_block[EnumDataBlockType::eManager];
    IDataBlock *envBlock = data_block[EnumDataBlockType::eEnv];
    IDataBlock *guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<Engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto env = dynamic_cast<Engine::Env *>(envBlock);
    assert(env != nullptr);

    auto gui = dynamic_cast<Engine::Panel *>(guiBlock);
    assert(gui != nullptr);

    auto context = Direct3D::GetInstance().device_context();

    float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    context->ClearRenderTargetView(
        Direct3D::GetInstance().render_target_view().Get(), clearColor);
    context->ClearDepthStencilView(
        Direct3D::GetInstance().depth_stencil_view().Get(),
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
            1, Direct3D::GetInstance().render_target_view().GetAddressOf(),
            Direct3D::GetInstance().depth_stencil_view().Get());
        context->OMSetDepthStencilState(
            Direct3D::GetInstance().depth_stencil_state().Get(), 0);

        context->VSSetShader(phong_shader->vertex_shader.Get(), 0, 0);
        context->PSSetSamplers(0, 1, &phong_shader->sample_state);

        if (gui->GetGlobalTab().draw_as_wire_)
            context->RSSetState(
                Direct3D::GetInstance().wire_rasterizer_state().Get());
        else
            context->RSSetState(
                Direct3D::GetInstance().solid_rasterizer_state().Get());

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
    IDataBlock *block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<Engine::PipelineManager *>(block);
    assert(manager != nullptr);

    manager->cube_map = std::make_shared<CubeMap>();
    GeometryGenerator::MakeBox(manager->cube_map.get());

    auto graph = std::make_shared<Graph>();
    graph->SetDetailNode(std::make_shared<GameObjectDetailNode>());
    manager->behaviors[manager->cube_map->GetEntityId()] = graph;

    auto cube_map_shader = std::make_shared<CubeMapShader>();
    manager->shaders[EnumShaderType::eCube] = cube_map_shader;

    std::reverse(manager->cube_map->mesh->indices.begin(),
                 manager->cube_map->mesh->indices.end());

    auto envFilename = L"./Assets/Textures/Cubemaps/HDRI/SampleEnvHDR.dds";
    auto specularFilename =
        L"./Assets/Textures/Cubemaps/HDRI/SampleSpecularHDR.dds";
    auto irradianceFilename =
        L"./Assets/Textures/Cubemaps/HDRI/SampleDiffuseHDR.dds";
    auto brdfFilename = L"./Assets/Textures/Cubemaps/HDRI/SampleBrdf.dds";

    cube_map_shader->CreateDDSTexture(envFilename, manager->cube_map->env_SRV);
    cube_map_shader->CreateDDSTexture(specularFilename,
                                      manager->cube_map->specular_SRV);
    cube_map_shader->CreateDDSTexture(irradianceFilename,
                                      manager->cube_map->irradiance_SRV);
    cube_map_shader->CreateDDSTexture(brdfFilename,
                                      manager->cube_map->brdf_SRV);

    manager->cube_map->cube_map_shader_source =
        std::make_shared<CubeMapShaderSource>();
    auto cube_map_shader_source = manager->cube_map->cube_map_shader_source;

    cube_map_shader_source->vertex_constant_buffer_data.model = Matrix();
    cube_map_shader_source->vertex_constant_buffer_data.view = Matrix();
    cube_map_shader_source->vertex_constant_buffer_data.projection = Matrix();

    Direct3D::GetInstance().CreateConstantBuffer(
        cube_map_shader_source->vertex_constant_buffer_data,
        cube_map_shader_source->vertex_constant_buffer);
    Direct3D::GetInstance().CreateConstantBuffer(
        cube_map_shader_source->pixel_constant_buffer_data,
        cube_map_shader_source->pixel_constant_buffer);

    Direct3D::GetInstance().CreateVertexBuffer(
        manager->cube_map->mesh->vertices,
        manager->cube_map->mesh->vertexBuffer);
    Direct3D::GetInstance().CreateIndexBuffer(
        manager->cube_map->mesh->indices, manager->cube_map->mesh->indexBuffer);

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
        L"cube_mapping_vertex_shader.hlsl", basicInputElements,
        cube_map_shader->vertex_shader, cube_map_shader->layout);

    Direct3D::GetInstance().CreatePixelShader(L"cube_mapping_pixel_shader.hlsl",
                                              cube_map_shader->pixel_shader);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus UpdateCubeMap::OnInvoke() {
    IDataBlock *managerBlock = data_block[EnumDataBlockType::eManager];
    IDataBlock *envBlock = data_block[EnumDataBlockType::eEnv];
    IDataBlock *guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<Engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto env = dynamic_cast<Engine::Env *>(envBlock);
    assert(env != nullptr);

    auto gui = dynamic_cast<Engine::Panel *>(guiBlock);
    assert(gui != nullptr);

    auto cube_map = manager->cube_map;

    auto cube_shader_source = cube_map->cube_map_shader_source;
    auto cube_map_shader = manager->shaders[EnumShaderType::eCube];

    cube_shader_source->pixel_constant_buffer_data.textureToDraw =
        gui->GetGlobalTab().cube_map_setting.textureToDraw;

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

    Direct3D::GetInstance().UpdateBuffer(
        cube_shader_source->vertex_constant_buffer_data,
        cube_shader_source->vertex_constant_buffer);

    Direct3D::GetInstance().UpdateBuffer(
        cube_shader_source->pixel_constant_buffer_data,
        cube_shader_source->pixel_constant_buffer);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus RenderCubeMap::OnInvoke() {
    IDataBlock *managerBlock = data_block[EnumDataBlockType::eManager];
    IDataBlock *envBlock = data_block[EnumDataBlockType::eEnv];
    IDataBlock *guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<Engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto env = dynamic_cast<Engine::Env *>(envBlock);
    assert(env != nullptr);

    auto gui = dynamic_cast<Engine::Panel *>(guiBlock);
    assert(gui != nullptr);

    auto context = Direct3D::GetInstance().device_context();
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

    std::vector<ID3D11ShaderResourceView *> srvs = {
        cube_map->env_SRV.Get(), cube_map->specular_SRV.Get(),
        cube_map->irradiance_SRV.Get()};
    context->PSSetShaderResources(0, UINT(srvs.size()), srvs.data());

    context->PSSetShader(cube_map_shader->pixel_shader.Get(), 0, 0);
    context->PSSetSamplers(0, 1, cube_map_shader->sample_state.GetAddressOf());

    context->DrawIndexed(cube_map->GetIndexCount(), 0, 0);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus InitializeImageBasedShader::OnInvoke() {
    IDataBlock *block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<Engine::PipelineManager *>(block);
    assert(manager != nullptr);

    // Create and initialize the light shader object.
    auto image_based_shader = std::make_shared<ImageBasedShader>();
    manager->shaders[EnumShaderType::eImageBased] = image_based_shader;

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
    Direct3D::GetInstance().device()->CreateSamplerState(
        &sampDesc, image_based_shader->sample_state.GetAddressOf());

    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 3,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    Direct3D::GetInstance().CreateVertexShaderAndInputLayout(
        L"image_based_vertex_shader.hlsl", inputElements,
        image_based_shader->vertex_shader, image_based_shader->layout);

    Direct3D::GetInstance().CreatePixelShader(L"image_based_pixel_shader.hlsl",
                                              image_based_shader->pixel_shader);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus UpdateGameObjectsUsingImageBasedShader::OnInvoke() {
    IDataBlock *managerBlock = data_block[EnumDataBlockType::eManager];
    IDataBlock *envBlock = data_block[EnumDataBlockType::eEnv];
    IDataBlock *guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<Engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto env = dynamic_cast<Engine::Env *>(envBlock);
    assert(env != nullptr);

    auto gui = dynamic_cast<Engine::Panel *>(guiBlock);
    assert(gui != nullptr);

    auto context = Direct3D::GetInstance().device_context();

    for (auto &model_amp : manager->models) {
        auto &model = model_amp.second;
        auto graph = manager->behaviors[model->GetEntityId()];

        auto detail = dynamic_cast<Engine::GameObjectDetailNode *>(
            graph->GetDetailNode().get());
        assert(detail != nullptr);

        auto image_based_shader_source = model->image_based_shader_source;
        auto image_based_shader = manager->shaders[EnumShaderType::eImageBased];
        // model
        {
            image_based_shader_source->vertex_constant_buffer_data.model =
                Matrix::CreateScale(detail->scaling) *
                Matrix::CreateRotationX(detail->rotation.x) *
                Matrix::CreateRotationY(detail->rotation.y) *
                Matrix::CreateRotationZ(detail->rotation.z) *
                Matrix::CreateTranslation(detail->translation);

            image_based_shader_source->vertex_constant_buffer_data.model =
                image_based_shader_source->vertex_constant_buffer_data.model
                    .Transpose();
        }
        // view
        {
            image_based_shader_source->vertex_constant_buffer_data.view =
                manager->camera->view.Transpose();
        }
        // inverse transpose
        {
            image_based_shader_source->vertex_constant_buffer_data
                .invTranspose =
                image_based_shader_source->vertex_constant_buffer_data.model;
            image_based_shader_source->vertex_constant_buffer_data.invTranspose
                .Translation(Vector3(0.0f));
            image_based_shader_source->vertex_constant_buffer_data
                .invTranspose =
                image_based_shader_source->vertex_constant_buffer_data
                    .invTranspose.Transpose()
                    .Invert();
        }
        // projection
        {
            const float aspect = env->aspect_;
            if (detail->use_perspective_projection) {
                image_based_shader_source->vertex_constant_buffer_data
                    .projection = XMMatrixPerspectiveFovLH(
                    XMConvertToRadians(
                        gui->GetGlobalTab()
                            .projection_setting.projection_fov_angle_y),
                    aspect, gui->GetGlobalTab().projection_setting.near_z,
                    gui->GetGlobalTab().projection_setting.far_z);
            } else {
                image_based_shader_source->vertex_constant_buffer_data
                    .projection = XMMatrixOrthographicOffCenterLH(
                    -aspect, aspect, -1.0f, 1.0f,
                    gui->GetGlobalTab().projection_setting.near_z,
                    gui->GetGlobalTab().projection_setting.far_z);
            }
            image_based_shader_source->vertex_constant_buffer_data.projection =
                image_based_shader_source->vertex_constant_buffer_data
                    .projection.Transpose();
        }

        Direct3D::GetInstance().UpdateBuffer(
            image_based_shader_source->vertex_constant_buffer_data,
            image_based_shader_source->vertex_constant_buffer);

        // eye
        {
            image_based_shader_source->pixel_constant_buffer_data.eyeWorld =
                Vector3::Transform(
                    Vector3(0.0f),
                    image_based_shader_source->vertex_constant_buffer_data.view
                        .Invert());
        }
        // material
        {
            image_based_shader_source->pixel_constant_buffer_data.material
                .diffuse = Vector3(detail->diffuse);
            image_based_shader_source->pixel_constant_buffer_data.material
                .specular = Vector3(detail->specular);
            image_based_shader_source->pixel_constant_buffer_data.material
                .shininess = detail->shininess;
        }

        image_based_shader_source->pixel_constant_buffer_data.useTexture =
            detail->use_texture;

        Direct3D::GetInstance().UpdateBuffer(
            image_based_shader_source->pixel_constant_buffer_data,
            image_based_shader_source->pixel_constant_buffer);
    }
    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus RenderGameObjectsUsingImageBasedShader::OnInvoke() {
    IDataBlock *managerBlock = data_block[EnumDataBlockType::eManager];
    IDataBlock *envBlock = data_block[EnumDataBlockType::eEnv];
    IDataBlock *guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<Engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto env = dynamic_cast<Engine::Env *>(envBlock);
    assert(env != nullptr);

    auto gui = dynamic_cast<Engine::Panel *>(guiBlock);
    assert(gui != nullptr);

    auto context = Direct3D::GetInstance().device_context();

    float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    context->ClearRenderTargetView(
        Direct3D::GetInstance().render_target_view().Get(), clearColor);
    context->ClearDepthStencilView(
        Direct3D::GetInstance().depth_stencil_view().Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    for (auto &model_map : manager->models) {
        // RS: Rasterizer stage
        // OM: Output-Merger stage
        // VS: Vertex Shader
        // PS: Pixel Shader
        // IA: Input-Assembler stage
        auto model = model_map.second;
        auto image_based_shader_source = model->image_based_shader_source;
        auto image_based_shader = manager->shaders[EnumShaderType::eImageBased];

        unsigned int stride = sizeof(Vertex);
        unsigned int offset = 0;

        context->OMSetRenderTargets(
            1, Direct3D::GetInstance().render_target_view().GetAddressOf(),
            Direct3D::GetInstance().depth_stencil_view().Get());
        context->OMSetDepthStencilState(
            Direct3D::GetInstance().depth_stencil_state().Get(), 0);

        context->VSSetShader(image_based_shader->vertex_shader.Get(), 0, 0);
        context->PSSetSamplers(0, 1, &image_based_shader->sample_state);

        if (gui->GetGlobalTab().draw_as_wire_)
            context->RSSetState(
                Direct3D::GetInstance().wire_rasterizer_state().Get());
        else
            context->RSSetState(
                Direct3D::GetInstance().solid_rasterizer_state().Get());

        context->VSSetConstantBuffers(
            0, 1,
            image_based_shader_source->vertex_constant_buffer.GetAddressOf());

        for (const auto &mesh : model->meshes) {

            std::vector<ID3D11ShaderResourceView *> resViews = {
                mesh->textureResourceView.Get(),
                manager->cube_map->specular_SRV.Get(),
                manager->cube_map->irradiance_SRV.Get(),
            };
            context->PSSetShaderResources(0, UINT(resViews.size()),
                                          resViews.data());

            /*std::vector<ID3D11ShaderResourceView *> resViews = {
                m_specularSRV.Get(),     m_irradianceSRV.Get(),
                m_brdfSRV.Get(),         mesh->albedoSRV.Get(),
                mesh->normalSRV.Get(),   mesh->aoSRV.Get(),
                mesh->metallicSRV.Get(), mesh->roughnessSRV.Get(),
                mesh->emissiveSRV.Get()};
            context->PSSetShaderResources(0, UINT(resViews.size()),
                                          resViews.data());*/

            context->PSSetConstantBuffers(
                0, 1,
                image_based_shader_source->pixel_constant_buffer
                    .GetAddressOf());
            context->PSSetShader(image_based_shader->pixel_shader.Get(), NULL,
                                 0);

            context->IASetInputLayout(image_based_shader->layout.Get());
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

EnumBehaviorTreeStatus CheckImageBasedShader::CheckCondition() {
    IDataBlock *guiBlock = data_block[EnumDataBlockType::eGui];
    auto gui = dynamic_cast<Engine::Panel *>(guiBlock);
    assert(gui != nullptr);

    if (gui->GetGlobalTab().render_mode ==
        EnumRenderMode::eImageBasedLighting) {
        return EnumBehaviorTreeStatus::eSuccess;
    }

    return EnumBehaviorTreeStatus::eFail;
}

EnumBehaviorTreeStatus CheckPhongShader::CheckCondition() {
    IDataBlock *guiBlock = data_block[EnumDataBlockType::eGui];
    auto gui = dynamic_cast<Engine::Panel *>(guiBlock);
    assert(gui != nullptr);

    if (gui->GetGlobalTab().render_mode == EnumRenderMode::eLight) {
        return EnumBehaviorTreeStatus::eSuccess;
    }

    return EnumBehaviorTreeStatus::eFail;
}

EnumBehaviorTreeStatus CheckPhysicallyBasedShader::CheckCondition() {
    IDataBlock *guiBlock = data_block[EnumDataBlockType::eGui];
    auto gui = dynamic_cast<Engine::Panel *>(guiBlock);
    assert(gui != nullptr);

    if (gui->GetGlobalTab().render_mode ==
        EnumRenderMode::ePhysicallyBasedRendering) {
        return EnumBehaviorTreeStatus::eSuccess;
    }

    return EnumBehaviorTreeStatus::eFail;
}

EnumBehaviorTreeStatus InitializePhysicallyBasedShader::OnInvoke() {
    IDataBlock *block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<Engine::PipelineManager *>(block);
    assert(manager != nullptr);

    // Create and initialize the light shader object.
    auto physical_shader = std::make_shared<PhsicallyBasedShader>();
    manager->shaders[EnumShaderType::ePhysicallyBased] = physical_shader;

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
    Direct3D::GetInstance().device()->CreateSamplerState(
        &sampDesc, physical_shader->sample_state.GetAddressOf());

    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

    Direct3D::GetInstance().device()->CreateSamplerState(
        &sampDesc, physical_shader->clampSamplerState.GetAddressOf());

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

    Direct3D::GetInstance().CreateVertexShaderAndInputLayout(
        L"physically_based_vertex_shader.hlsl", inputElements,
        physical_shader->vertex_shader, physical_shader->layout);

    Direct3D::GetInstance().CreatePixelShader(
        L"physically_based_pixel_shader.hlsl", physical_shader->pixel_shader);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus InitializeNormalGeometryShader::OnInvoke() {
    IDataBlock *block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<Engine::PipelineManager *>(block);
    assert(manager != nullptr);

    // Create and initialize the light shader object.
    auto geometry_shader = std::make_shared<NormalGeometryShader>();
    manager->shaders[EnumShaderType::eNormalGeometry] = geometry_shader;

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
    Direct3D::GetInstance().device()->CreateSamplerState(
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

    Direct3D::GetInstance().CreateVertexShaderAndInputLayout(
        L"physically_vertex_shader.hlsl", inputElements,
        geometry_shader->vertex_shader, geometry_shader->layout);

    // Geometry shader �ʱ�ȭ�ϱ�
    Direct3D::GetInstance().CreateGeometryShader(
        L"NormalGS.hlsl", geometry_shader->normalGeometryShader);

    Direct3D::GetInstance().CreateVertexShaderAndInputLayout(
        L"NormalVS.hlsl", inputElements, geometry_shader->vertex_shader,
        geometry_shader->layout);

    Direct3D::GetInstance().CreatePixelShader(L"NormalPS.hlsl",
                                              geometry_shader->pixel_shader);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus UpdateGameObjectsUsingPhysicallyBasedShader::OnInvoke() {
    IDataBlock *managerBlock = data_block[EnumDataBlockType::eManager];
    IDataBlock *envBlock = data_block[EnumDataBlockType::eEnv];
    IDataBlock *guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<Engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto env = dynamic_cast<Engine::Env *>(envBlock);
    assert(env != nullptr);

    auto gui = dynamic_cast<Engine::Panel *>(guiBlock);
    assert(gui != nullptr);

    auto context = Direct3D::GetInstance().device_context();

    for (auto &model_amp : manager->models) {
        auto &model = model_amp.second;
        auto graph = manager->behaviors[model->GetEntityId()];

        auto detail = dynamic_cast<Engine::GameObjectDetailNode *>(
            graph->GetDetailNode().get());
        assert(detail != nullptr);

        auto physically_shader_source = model->physically_based_shader_source;
        auto physically_shader =
            manager->shaders[EnumShaderType::ePhysicallyBased];
        // model
        {
            physically_shader_source->vertex_constant_buffer_data.modelWorld =
                Matrix::CreateScale(detail->scaling) *
                Matrix::CreateRotationX(detail->rotation.x) *
                Matrix::CreateRotationY(detail->rotation.y) *
                Matrix::CreateRotationZ(detail->rotation.z) *
                Matrix::CreateTranslation(detail->translation);

            physically_shader_source->vertex_constant_buffer_data.modelWorld =
                physically_shader_source->vertex_constant_buffer_data.modelWorld
                    .Transpose();
        }
        // view
        {
            physically_shader_source->vertex_constant_buffer_data.view =
                manager->camera->view.Transpose();
        }
        // inverse transpose
        {
            physically_shader_source->vertex_constant_buffer_data.invTranspose =
                physically_shader_source->vertex_constant_buffer_data
                    .modelWorld;
            physically_shader_source->vertex_constant_buffer_data.invTranspose
                .Translation(Vector3(0.0f));
            physically_shader_source->vertex_constant_buffer_data.invTranspose =
                physically_shader_source->vertex_constant_buffer_data
                    .invTranspose.Transpose()
                    .Invert();
        }
        // projection
        {
            const float aspect = env->aspect_;
            if (detail->use_perspective_projection) {
                physically_shader_source->vertex_constant_buffer_data
                    .projection = XMMatrixPerspectiveFovLH(
                    XMConvertToRadians(
                        gui->GetGlobalTab()
                            .projection_setting.projection_fov_angle_y),
                    aspect, gui->GetGlobalTab().projection_setting.near_z,
                    gui->GetGlobalTab().projection_setting.far_z);
            } else {
                physically_shader_source->vertex_constant_buffer_data
                    .projection = XMMatrixOrthographicOffCenterLH(
                    -aspect, aspect, -1.0f, 1.0f,
                    gui->GetGlobalTab().projection_setting.near_z,
                    gui->GetGlobalTab().projection_setting.far_z);
            }
            physically_shader_source->vertex_constant_buffer_data.projection =
                physically_shader_source->vertex_constant_buffer_data.projection
                    .Transpose();
        }

        Direct3D::GetInstance().UpdateBuffer(
            physically_shader_source->vertex_constant_buffer_data,
            physically_shader_source->vertex_constant_buffer);

        // eye
        {
            physically_shader_source->pixel_constant_buffer_data.eyeWorld =
                Vector3::Transform(
                    Vector3(0.0f),
                    physically_shader_source->vertex_constant_buffer_data.view
                        .Invert());
        }
        // material
        {
            physically_shader_source->pixel_constant_buffer_data.material
                .roughness = gui->GetGlobalTab().pbr_setting.roughness;
            physically_shader_source->pixel_constant_buffer_data.material
                .metallic = gui->GetGlobalTab().pbr_setting.metallic;
        }
        // light
        {
            for (int i = 0; i < MAX_LIGHTS; i++) {
                if (i != gui->GetGlobalTab().light_setting.light_type) {
                    physically_shader_source->pixel_constant_buffer_data
                        .lights[i]
                        .strength *= 0.0f;
                } else {
                    // turn off another light
                    physically_shader_source->pixel_constant_buffer_data
                        .lights[i] =
                        gui->GetGlobalTab().light_setting.light_from_gui;
                }
            }
        }

        physically_shader_source->pixel_constant_buffer_data.useAlbedoMap =
            gui->GetGlobalTab().pbr_setting.useAlbedoMap;
        physically_shader_source->pixel_constant_buffer_data.useNormalMap =
            gui->GetGlobalTab().pbr_setting.useNormalMap;
        physically_shader_source->pixel_constant_buffer_data.useAOMap =
            gui->GetGlobalTab().pbr_setting.useAOMap;
        physically_shader_source->pixel_constant_buffer_data.invertNormalMapY =
            gui->GetGlobalTab().pbr_setting.invertNormalMapY;
        physically_shader_source->pixel_constant_buffer_data.useMetallicMap =
            gui->GetGlobalTab().pbr_setting.useMetallicMap;
        physically_shader_source->pixel_constant_buffer_data.useRoughnessMap =
            gui->GetGlobalTab().pbr_setting.useRoughnessMap;
        physically_shader_source->pixel_constant_buffer_data.useEmissiveMap =
            gui->GetGlobalTab().pbr_setting.useEmissiveMap;

        Direct3D::GetInstance().UpdateBuffer(
            physically_shader_source->pixel_constant_buffer_data,
            physically_shader_source->pixel_constant_buffer);
    }
    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus RenderGameObjectsUsingPhysicallyBasedShader::OnInvoke() {
    IDataBlock *managerBlock = data_block[EnumDataBlockType::eManager];
    IDataBlock *envBlock = data_block[EnumDataBlockType::eEnv];
    IDataBlock *guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<Engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto env = dynamic_cast<Engine::Env *>(envBlock);
    assert(env != nullptr);

    auto gui = dynamic_cast<Engine::Panel *>(guiBlock);
    assert(gui != nullptr);

    auto context = Direct3D::GetInstance().device_context();

    float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    context->ClearRenderTargetView(
        Direct3D::GetInstance().render_target_view().Get(), clearColor);
    context->ClearDepthStencilView(
        Direct3D::GetInstance().depth_stencil_view().Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    for (auto &model_map : manager->models) {
        // RS: Rasterizer stage
        // OM: Output-Merger stage
        // VS: Vertex Shader
        // PS: Pixel Shader
        // IA: Input-Assembler stage
        auto model = model_map.second;
        auto physically_shader_source = model->physically_based_shader_source;
        auto physically_shader = std::static_pointer_cast<PhsicallyBasedShader>(
            manager->shaders[EnumShaderType::ePhysicallyBased]);

        assert(physically_shader != nullptr);

        unsigned int stride = sizeof(Vertex);
        unsigned int offset = 0;

        std::vector<ID3D11RenderTargetView *> renderTargetViews = {
            Direct3D::GetInstance().render_target_view().Get()};

        context->OMSetRenderTargets(
            UINT(renderTargetViews.size()), renderTargetViews.data(),
            Direct3D::GetInstance().depth_stencil_view().Get());

        context->OMSetDepthStencilState(
            Direct3D::GetInstance().depth_stencil_state().Get(), 0);

        if (gui->GetGlobalTab().draw_as_wire_)
            context->RSSetState(
                Direct3D::GetInstance().wire_rasterizer_state().Get());
        else
            context->RSSetState(
                Direct3D::GetInstance().solid_rasterizer_state().Get());

        for (const auto &mesh : model->meshes) {

            // VertexShader������ Texture ���
            context->VSSetShader(physically_shader->vertex_shader.Get(), 0, 0);
            context->VSSetShaderResources(0, 1, mesh->heightSRV.GetAddressOf());
            context->VSSetSamplers(
                0, 1, physically_shader->sample_state.GetAddressOf());
            context->VSSetConstantBuffers(
                0, 1,
                physically_shader_source->vertex_constant_buffer
                    .GetAddressOf());

            std::vector<ID3D11SamplerState *> samplers = {
                physically_shader->sample_state.Get(),
                physically_shader->clampSamplerState.Get()};

            context->PSSetSamplers(0, UINT(samplers.size()), samplers.data());
            context->PSSetShader(physically_shader->pixel_shader.Get(), 0, 0);

            std::vector<ID3D11ShaderResourceView *> resViews = {
                manager->cube_map->specular_SRV.Get(),
                manager->cube_map->irradiance_SRV.Get(),
                manager->cube_map->brdf_SRV.Get(),
                mesh->albedoSRV.Get(),
                mesh->normalSRV.Get(),
                mesh->aoSRV.Get(),
                mesh->metallicSRV.Get(),
                mesh->roughnessSRV.Get(),
                mesh->emissiveSRV.Get()};
            context->PSSetShaderResources(0, UINT(resViews.size()),
                                          resViews.data());

            context->PSSetConstantBuffers(
                0, 1,
                physically_shader_source->pixel_constant_buffer.GetAddressOf());

            context->IASetInputLayout(physically_shader->layout.Get());
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
