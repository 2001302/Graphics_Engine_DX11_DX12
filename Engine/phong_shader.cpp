#include "phong_shader.h"
#include "panel.h"

using namespace Engine;

    void PhongShaderSource::InitializeThis() {
        // create constant buffer(Phong Shader)
        vertex_constant_buffer_data.model = Matrix();
        vertex_constant_buffer_data.view = Matrix();
        vertex_constant_buffer_data.projection = Matrix();

        Direct3D::GetInstance().CreateConstantBuffer(
            vertex_constant_buffer_data, vertex_constant_buffer);
        Direct3D::GetInstance().CreateConstantBuffer(pixel_constant_buffer_data,
                                                     pixel_constant_buffer);
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

EnumBehaviorTreeStatus InitializePhongShader::OnInvoke() {
    IDataBlock *block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<Engine::PipelineManager *>(block);
    assert(manager != nullptr);

    // Create and initialize the light shader object.
    auto phong_shader = std::make_shared<PhongShader>();
    manager->shaders[EnumShaderType::ePhong] = phong_shader;

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
        auto graph = manager->models[model->GetEntityId()]->behavior;

        auto detail = dynamic_cast<Engine::ModelDetailNode *>(
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
