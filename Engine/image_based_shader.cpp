#include "image_based_shader.h"
#include "geometry_generator.h"
#include "setting_ui.h"
#include "pipeline_manager.h"

using namespace dx11;

void ImageBasedShaderSource::InitializeThis() {
    vertex_constant_buffer_data.model = Matrix();
    vertex_constant_buffer_data.view = Matrix();
    vertex_constant_buffer_data.projection = Matrix();

    Direct3D::Instance().CreateConstantBuffer(vertex_constant_buffer_data,
                                                 vertex_constant_buffer);
    Direct3D::Instance().CreateConstantBuffer(pixel_constant_buffer_data,
                                                 pixel_constant_buffer);
}

EnumBehaviorTreeStatus InitializeImageBasedShader::OnInvoke() {
    auto block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<dx11::PipelineManager *>(block);
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
    Direct3D::Instance().device()->CreateSamplerState(
        &sampDesc, image_based_shader->sample_state.GetAddressOf());

    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 3,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    Direct3D::Instance().CreateVertexShaderAndInputLayout(
        L"image_based_vertex_shader.hlsl", inputElements,
        image_based_shader->vertex_shader, image_based_shader->layout);

    Direct3D::Instance().CreatePixelShader(L"image_based_pixel_shader.hlsl",
                                              image_based_shader->pixel_shader);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus CheckImageBasedShader::CheckCondition() {
    auto guiBlock = data_block[EnumDataBlockType::eGui];
    auto gui = dynamic_cast<common::SettingUi *>(guiBlock);
    assert(gui != nullptr);

    if (gui->GetGlobalTab().common_setting.render_mode ==
        common::EnumRenderMode::eImageBasedLighting) {

        return EnumBehaviorTreeStatus::eSuccess;
    }

    return EnumBehaviorTreeStatus::eFail;
}

EnumBehaviorTreeStatus UpdateGameObjectsUsingImageBasedShader::OnInvoke() {
    auto managerBlock = data_block[EnumDataBlockType::eManager];
    auto guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<dx11::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto gui = dynamic_cast<common::SettingUi *>(guiBlock);
    assert(gui != nullptr);

    auto context = Direct3D::Instance().device_context();

    auto model = dynamic_cast<Model *>(manager->models[target_id].get());

    auto image_based_shader = manager->shaders[EnumShaderType::eImageBased];
    if (image_based_shader->source[target_id] == nullptr) {

        auto source = std::make_shared<ImageBasedShaderSource>();
        source->Initialize();
        image_based_shader->source[target_id] = source;
    }
    auto image_based_shader_source = dynamic_cast<ImageBasedShaderSource *>(
        image_based_shader->source[model->GetEntityId()].get());

    // model
    {
        image_based_shader_source->vertex_constant_buffer_data.model =
            Matrix::CreateScale(model->scaling) *
            Matrix::CreateRotationX(model->rotation.x) *
            Matrix::CreateRotationY(model->rotation.y) *
            Matrix::CreateRotationZ(model->rotation.z) *
            Matrix::CreateTranslation(model->translation);

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
        image_based_shader_source->vertex_constant_buffer_data.invTranspose =
            image_based_shader_source->vertex_constant_buffer_data.model;
        image_based_shader_source->vertex_constant_buffer_data.invTranspose
            .Translation(Vector3(0.0f));
        image_based_shader_source->vertex_constant_buffer_data.invTranspose =
            image_based_shader_source->vertex_constant_buffer_data.invTranspose
                .Transpose()
                .Invert();
    }
    // projection
    {
        const float aspect = common::Env::Instance().aspect;
        image_based_shader_source->vertex_constant_buffer_data
            .projection = XMMatrixPerspectiveFovLH(
            XMConvertToRadians(
                gui->GetGlobalTab().projection_setting.projection_fov_angle_y),
            aspect, gui->GetGlobalTab().projection_setting.near_z,
            gui->GetGlobalTab().projection_setting.far_z);

        image_based_shader_source->vertex_constant_buffer_data.projection =
            image_based_shader_source->vertex_constant_buffer_data.projection
                .Transpose();
    }

    Direct3D::Instance().UpdateBuffer(
        image_based_shader_source->vertex_constant_buffer_data,
        image_based_shader_source->vertex_constant_buffer);

    // eye
    {
        image_based_shader_source->pixel_constant_buffer_data.eyeWorld =
            Vector3::Transform(Vector3(0.0f),
                               image_based_shader_source
                                   ->vertex_constant_buffer_data.view.Invert());
    }
    //// material
    //{
    //    image_based_shader_source->pixel_constant_buffer_data.material.diffuse
    //    =
    //        Vector3(detail->diffuse);
    //    image_based_shader_source->pixel_constant_buffer_data.material
    //        .specular = Vector3(detail->specular);
    //    image_based_shader_source->pixel_constant_buffer_data.material
    //        .shininess = detail->shininess;
    //}

    // image_based_shader_source->pixel_constant_buffer_data.useTexture =
    //     detail->use_texture;

    Direct3D::Instance().UpdateBuffer(
        image_based_shader_source->pixel_constant_buffer_data,
        image_based_shader_source->pixel_constant_buffer);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus RenderGameObjectsUsingImageBasedShader::OnInvoke() {
    auto managerBlock = data_block[EnumDataBlockType::eManager];
    auto guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<dx11::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto gui = dynamic_cast<common::SettingUi *>(guiBlock);
    assert(gui != nullptr);

    auto context = Direct3D::Instance().device_context();

    // RS: Rasterizer stage
    // OM: Output-Merger stage
    // VS: Vertex Shader
    // PS: Pixel Shader
    // IA: Input-Assembler stage
    auto model = dynamic_cast<Model *>(manager->models[target_id].get());
    auto image_based_shader = manager->shaders[EnumShaderType::eImageBased];
    auto image_based_shader_source = dynamic_cast<ImageBasedShaderSource *>(
        image_based_shader->source[model->GetEntityId()].get());

    unsigned int stride = sizeof(Vertex);
    unsigned int offset = 0;

    context->VSSetShader(image_based_shader->vertex_shader.Get(), 0, 0);
    context->PSSetSamplers(0, 1, &image_based_shader->sample_state);

    context->VSSetConstantBuffers(
        0, 1, image_based_shader_source->vertex_constant_buffer.GetAddressOf());

    context->PSSetConstantBuffers(
        0, 1, image_based_shader_source->pixel_constant_buffer.GetAddressOf());
    context->PSSetShader(image_based_shader->pixel_shader.Get(), NULL, 0);
    context->IASetInputLayout(image_based_shader->layout.Get());

    for (const auto &mesh : model->meshes) {

        std::vector<ID3D11ShaderResourceView *> resViews = {
            mesh->textureResourceView.Get(),
            manager->cube_map->texture->specular_SRV.Get(),
            manager->cube_map->texture->irradiance_SRV.Get(),
        };
        context->PSSetShaderResources(0, UINT(resViews.size()),
                                      resViews.data());

        context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(),
                                    &stride, &offset);
        context->IASetIndexBuffer(mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT,
                                  0);
    }

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->DrawIndexed(model->GetIndexCount(), 0, 0);

    return EnumBehaviorTreeStatus::eSuccess;
}
