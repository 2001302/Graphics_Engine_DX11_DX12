#include "render_node.h"
#include "game_object_node.h"
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
        DirectX::SimpleMath::Vector3(-50.0f, 0.0f, -50.0f);
    manager->camera->rotation = DirectX::SimpleMath::Vector3(0.0f, 45.0f, 0.0f);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus InitializePhongShader::OnInvoke() {
    IDataBlock *block = DataBlock[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<Engine::PipelineManager *>(block);
    assert(manager != nullptr);

    // Create and initialize the light shader object.
    manager->phongShader = std::make_unique<PhongShader>();

    // Texture sampler ¸¸µé±â
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
        &sampDesc, manager->phongShader->sample_state.GetAddressOf());

    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 3,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    manager->phongShader->CreateVertexShaderAndInputLayout(
        L"phong_vertex_shader.hlsl", inputElements,
        manager->phongShader->vertex_shader, manager->phongShader->layout);

    manager->phongShader->CreatePixelShader(L"phong_pixel_shader.hlsl",
                                            manager->phongShader->pixel_shader);

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

        // model
        {
            model->phongShader->vertex_constant_buffer_data.model =
                Matrix::CreateScale(detail->scaling) *
                Matrix::CreateRotationX(detail->rotation.x) *
                Matrix::CreateRotationY(detail->rotation.y) *
                Matrix::CreateRotationZ(detail->rotation.z) *
                Matrix::CreateTranslation(detail->translation);

            model->phongShader->vertex_constant_buffer_data.model =
                model->phongShader->vertex_constant_buffer_data.model
                    .Transpose();
        }
        // view
        {
            model->phongShader->vertex_constant_buffer_data.view =
                manager->camera->view.Transpose();
        }
        // inverse transpose
        {
            model->phongShader->vertex_constant_buffer_data.invTranspose =
                model->phongShader->vertex_constant_buffer_data.model;
            model->phongShader->vertex_constant_buffer_data.invTranspose
                .Translation(Vector3(0.0f));
            model->phongShader->vertex_constant_buffer_data.invTranspose =
                model->phongShader->vertex_constant_buffer_data.invTranspose
                    .Transpose()
                    .Invert();
        }
        // projection
        {
            const float aspect = env->aspect_;
            if (detail->use_perspective_projection) {
                model->phongShader->vertex_constant_buffer_data.projection =
                    XMMatrixPerspectiveFovLH(
                        XMConvertToRadians(gui->projection_fov_angle_y), aspect,
                        gui->near_z, gui->far_z);
            } else {
                model->phongShader->vertex_constant_buffer_data.projection =
                    XMMatrixOrthographicOffCenterLH(-aspect, aspect, -1.0f,
                                                    1.0f, gui->near_z,
                                                    gui->far_z);
            }
            model->phongShader->vertex_constant_buffer_data.projection =
                model->phongShader->vertex_constant_buffer_data.projection
                    .Transpose();
        }

        manager->phongShader->UpdateBuffer(
            model->phongShader->vertex_constant_buffer_data,
            model->phongShader->vertex_constant_buffer);

        // eye
        {
            model->phongShader->pixel_constant_buffer_data
                .eyeWorld = Vector3::Transform(
                Vector3(0.0f),
                model->phongShader->vertex_constant_buffer_data.view.Invert());
        }
        // material
        {
            model->phongShader->pixel_constant_buffer_data.material.diffuse =
                Vector3(detail->diffuse);
            model->phongShader->pixel_constant_buffer_data.material.specular =
                Vector3(detail->specular);
            model->phongShader->pixel_constant_buffer_data.material.shininess =
                detail->shininess;
        }
        // light
        {
            for (int i = 0; i < MAX_LIGHTS; i++) {
                if (i != gui->light_type) {
                    model->phongShader->pixel_constant_buffer_data.lights[i]
                        .strength *= 0.0f;
                } else {
                    // turn off another light
                    model->phongShader->pixel_constant_buffer_data.lights[i] =
                        gui->light_from_gui;
                }
            }
        }

        model->phongShader->pixel_constant_buffer_data.useTexture =
            detail->use_texture;
        model->phongShader->pixel_constant_buffer_data.useBlinnPhong =
            detail->use_blinn_phong;

        manager->phongShader->UpdateBuffer(
            model->phongShader->pixel_constant_buffer_data,
            model->phongShader->pixel_constant_buffer);
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

        unsigned int stride = sizeof(Vertex);
        unsigned int offset = 0;

        context->OMSetRenderTargets(
            1, &Direct3D::GetInstance().render_target_view_,
            Direct3D::GetInstance().depth_stencil_view_.Get());
        context->OMSetDepthStencilState(
            Direct3D::GetInstance().depth_stencil_state_.Get(), 0);

        context->VSSetShader(manager->phongShader->vertex_shader.Get(), 0, 0);
        context->PSSetSamplers(0, 1, &manager->phongShader->sample_state);

        if (gui->draw_as_wire_)
            context->RSSetState(
                Direct3D::GetInstance().wire_rasterizer_state_.Get());
        else
            context->RSSetState(
                Direct3D::GetInstance().solid_rasterizer_state_.Get());

        context->VSSetConstantBuffers(
            0, 1, model->phongShader->vertex_constant_buffer.GetAddressOf());

        for (const auto &mesh : model->meshes) {
            context->PSSetShaderResources(
                0, 1, mesh->textureResourceView.GetAddressOf());

            context->PSSetConstantBuffers(
                0, 1, model->phongShader->pixel_constant_buffer.GetAddressOf());
            context->PSSetShader(manager->phongShader->pixel_shader.Get(), NULL,
                                 0);

            context->IASetInputLayout(manager->phongShader->layout.Get());
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