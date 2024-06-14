#include "behavior_leaf.h"
#include "panel.h"
#include "pipeline_manager.h"

using namespace Engine;

EnumBehaviorTreeStatus InitializeCamera::Invoke() {
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

EnumBehaviorTreeStatus InitializePhongShader::Invoke() {
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

EnumBehaviorTreeStatus RenderGameObjects::Invoke() {
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

    // Generate the view matrix based on the camera's position.
    manager->camera->Render();

    for (auto &model : manager->models) {
        // model
        {
            model->phongShader->vertex_constant_buffer_data.model =
                Matrix::CreateScale(gui->m_modelScaling) *
                Matrix::CreateRotationX(gui->m_modelRotation.x) *
                Matrix::CreateRotationY(gui->m_modelRotation.y) *
                Matrix::CreateRotationZ(gui->m_modelRotation.z) *
                Matrix::CreateTranslation(gui->m_modelTranslation);

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
            if (gui->m_usePerspectiveProjection) {
                model->phongShader->vertex_constant_buffer_data.projection =
                    XMMatrixPerspectiveFovLH(
                        XMConvertToRadians(gui->m_projFovAngleY), aspect,
                        gui->m_nearZ, gui->m_farZ);
            } else {
                model->phongShader->vertex_constant_buffer_data.projection =
                    XMMatrixOrthographicOffCenterLH(-aspect, aspect, -1.0f,
                                                    1.0f, gui->m_nearZ,
                                                    gui->m_farZ);
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
                Vector3(gui->m_materialDiffuse);
            model->phongShader->pixel_constant_buffer_data.material.specular =
                Vector3(gui->m_materialSpecular);
            model->phongShader->pixel_constant_buffer_data.material.shininess =
                gui->m_shininess;
        }
        // light
        {
            for (int i = 0; i < MAX_LIGHTS; i++) {
                if (i != gui->m_lightType) {
                    model->phongShader->pixel_constant_buffer_data.lights[i]
                        .strength *= 0.0f;
                } else {
                    // turn off another light
                    model->phongShader->pixel_constant_buffer_data.lights[i] =
                        gui->m_lightFromGUI;
                }
            }
        }

        model->phongShader->pixel_constant_buffer_data.useTexture =
            gui->m_useTexture;
        model->phongShader->pixel_constant_buffer_data.useBlinnPhong =
            gui->m_useBlinnPhong;

        manager->phongShader->UpdateBuffer(
            model->phongShader->pixel_constant_buffer_data,
            model->phongShader->pixel_constant_buffer);

        // RS: Rasterizer stage
        // OM: Output-Merger stage
        // VS: Vertex Shader
        // PS: Pixel Shader
        // IA: Input-Assembler stage

        unsigned int stride = sizeof(Vertex);
        unsigned int offset = 0;

        float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        context->ClearRenderTargetView(
            Direct3D::GetInstance().render_target_view_, clearColor);
        context->ClearDepthStencilView(
            Direct3D::GetInstance().depth_stencil_view_.Get(),
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        context->OMSetRenderTargets(
            1, &Direct3D::GetInstance().render_target_view_,
            Direct3D::GetInstance().depth_stencil_view_.Get());
        context->OMSetDepthStencilState(
            Direct3D::GetInstance().depth_stencil_state_.Get(), 0);

        context->VSSetShader(manager->phongShader->vertex_shader.Get(), 0, 0);
        context->PSSetSamplers(0, 1, &manager->phongShader->sample_state);

        if (gui->m_drawAsWire)
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