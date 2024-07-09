#include "image_based_shader.h"
#include "geometry_generator.h"
#include "panel.h"

using namespace Engine;

void ImageBasedShaderSource::InitializeThis() {
    vertex_constant_buffer_data.model = Matrix();
    vertex_constant_buffer_data.view = Matrix();
    vertex_constant_buffer_data.projection = Matrix();

    Direct3D::GetInstance().CreateConstantBuffer(vertex_constant_buffer_data,
                                                 vertex_constant_buffer);
    Direct3D::GetInstance().CreateConstantBuffer(pixel_constant_buffer_data,
                                                 pixel_constant_buffer);
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

EnumBehaviorTreeStatus InitializeImageBasedShader::OnInvoke() {
    IDataBlock *block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<Engine::PipelineManager *>(block);
    assert(manager != nullptr);

    // Create and initialize the light shader object.
    auto image_based_shader = std::make_shared<ImageBasedShader>();
    manager->shaders[EnumShaderType::eImageBased] = image_based_shader;

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
    IDataBlock *guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<Engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto gui = dynamic_cast<Engine::Panel *>(guiBlock);
    assert(gui != nullptr);

    auto context = Direct3D::GetInstance().device_context();

    for (auto &model_map : manager->models) {
        auto &model = model_map.second;
        auto graph = model->behavior;

        auto detail = dynamic_cast<Engine::ModelDetailNode *>(
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
            const float aspect = Env::Get().aspect;
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
    IDataBlock *guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<Engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

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
