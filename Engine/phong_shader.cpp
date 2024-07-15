#include "phong_shader.h"
#include "pipeline_manager.h"
#include "setting_ui.h"

using namespace dx11;

void PhongShaderSource::InitializeThis() {
    // create constant buffer(Phong Shader)
    vertex_constant_buffer_data.model = Matrix();
    vertex_constant_buffer_data.view = Matrix();
    vertex_constant_buffer_data.projection = Matrix();

    GraphicsContext::Instance().CreateConstantBuffer(
        vertex_constant_buffer_data, vertex_constant_buffer);
    GraphicsContext::Instance().CreateConstantBuffer(pixel_constant_buffer_data,
                                                     pixel_constant_buffer);
}

void PhongShaderSource::OnShow() {

    ImGui::Checkbox("Use Texture", &pixel_constant_buffer_data.useTexture);
    ImGui::Checkbox("Use BlinnPhong",
                    &pixel_constant_buffer_data.useBlinnPhong);

    ImGui::Text("Material");
    ImGui::SliderFloat("Shininess",
                       &pixel_constant_buffer_data.material.shininess, 0.01f,
                       1.0f);

    ImGui::SliderFloat(
        "Diffuse", &pixel_constant_buffer_data.material.diffuse.x, 0.0f, 1.0f);
    pixel_constant_buffer_data.material.diffuse =
        Vector3(pixel_constant_buffer_data.material.diffuse.x);

    ImGui::SliderFloat("Specular",
                       &pixel_constant_buffer_data.material.specular.x, 0.0f,
                       1.0f);
    pixel_constant_buffer_data.material.specular =
        Vector3(pixel_constant_buffer_data.material.specular);
};

EnumBehaviorTreeStatus InitializePhongShader::OnInvoke() {
    auto block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<dx11::PipelineManager *>(block);
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
    GraphicsContext::Instance().device()->CreateSamplerState(
        &sampDesc, phong_shader->sample_state.GetAddressOf());

    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 3,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    GraphicsContext::Instance().CreateVertexShaderAndInputLayout(
        L"phong_vertex_shader.hlsl", inputElements, phong_shader->vertex_shader,
        phong_shader->layout);

    GraphicsContext::Instance().CreatePixelShader(L"phong_pixel_shader.hlsl",
                                                  phong_shader->pixel_shader);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus CheckPhongShader::CheckCondition() {

    auto guiBlock = data_block[EnumDataBlockType::eGui];
    auto gui = dynamic_cast<common::SettingUi *>(guiBlock);
    assert(gui != nullptr);

    if (gui->Tab().common.render_mode == common::EnumRenderMode::eLight) {
        return EnumBehaviorTreeStatus::eSuccess;
    }

    return EnumBehaviorTreeStatus::eFail;
}

EnumBehaviorTreeStatus UpdateGameObjectsUsingPhongShader::OnInvoke() {
    auto managerBlock = data_block[EnumDataBlockType::eManager];
    auto guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<dx11::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto gui = dynamic_cast<common::SettingUi *>(guiBlock);
    assert(gui != nullptr);

    auto context = GraphicsContext::Instance().device_context();

    auto model = dynamic_cast<Model *>(manager->models[target_id].get());

    auto phong_shader = manager->shaders[EnumShaderType::ePhong];
    if (phong_shader->source[target_id] == nullptr) {

        auto source = std::make_shared<PhongShaderSource>();
        source->Initialize();
        phong_shader->source[target_id] = source;
    }
    auto phong_shader_source = dynamic_cast<PhongShaderSource *>(
        phong_shader->source[model->GetEntityId()].get());

    // model
    {
        phong_shader_source->vertex_constant_buffer_data.model =
            Matrix::CreateScale(model->scaling) *
            Matrix::CreateRotationX(model->rotation.x) *
            Matrix::CreateRotationY(model->rotation.y) *
            Matrix::CreateRotationZ(model->rotation.z) *
            Matrix::CreateTranslation(model->translation);

        phong_shader_source->vertex_constant_buffer_data.model =
            phong_shader_source->vertex_constant_buffer_data.model.Transpose();
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
        const float aspect = common::Env::Instance().aspect;

        phong_shader_source->vertex_constant_buffer_data.projection =
            XMMatrixPerspectiveFovLH(
                XMConvertToRadians(
                    gui->Tab().projection.projection_fov_angle_y),
                aspect, gui->Tab().projection.near_z,
                gui->Tab().projection.far_z);

        phong_shader_source->vertex_constant_buffer_data.projection =
            phong_shader_source->vertex_constant_buffer_data.projection
                .Transpose();
    }

    GraphicsContext::Instance().UpdateBuffer(
        phong_shader_source->vertex_constant_buffer_data,
        phong_shader_source->vertex_constant_buffer);

    // eye
    {
        phong_shader_source->pixel_constant_buffer_data.eyeWorld =
            Vector3::Transform(
                Vector3(0.0f),
                phong_shader_source->vertex_constant_buffer_data.view.Invert());
    }
    //// material
    //{
    //    phong_shader_source->pixel_constant_buffer_data.material.diffuse =
    //        Vector3(detail->diffuse);
    //    phong_shader_source->pixel_constant_buffer_data.material.specular =
    //        Vector3(detail->specular);
    //    phong_shader_source->pixel_constant_buffer_data.material.shininess =
    //        detail->shininess;
    //}
    // light
    {
        for (int i = 0; i < MAX_LIGHTS; i++) {
            if (i != gui->Tab().light.light_type) {
                phong_shader_source->pixel_constant_buffer_data.lights[i]
                    .strength *= 0.0f;
            } else {
                // turn off another light
                phong_shader_source->pixel_constant_buffer_data.lights[i] =
                    gui->Tab().light.light_from_gui;
            }
        }
    }

    // phong_shader_source->pixel_constant_buffer_data.useTexture =
    //     detail->use_texture;
    // phong_shader_source->pixel_constant_buffer_data.useBlinnPhong =
    //    gui->Tab().light.use_blinn_phong;

    GraphicsContext::Instance().UpdateBuffer(
        phong_shader_source->pixel_constant_buffer_data,
        phong_shader_source->pixel_constant_buffer);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus RenderGameObjectsUsingPhongShader::OnInvoke() {
    auto managerBlock = data_block[EnumDataBlockType::eManager];
    auto guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<dx11::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto gui = dynamic_cast<common::SettingUi *>(guiBlock);
    assert(gui != nullptr);

    auto context = GraphicsContext::Instance().device_context();

    // RS: Rasterizer stage
    // OM: Output-Merger stage
    // VS: Vertex Shader
    // PS: Pixel Shader
    // IA: Input-Assembler stage

    auto model = dynamic_cast<Model *>(manager->models[target_id].get());
    auto phong_shader = manager->shaders[EnumShaderType::ePhong];
    auto phong_shader_source = dynamic_cast<PhongShaderSource *>(
        phong_shader->source[model->GetEntityId()].get());

    unsigned int stride = sizeof(Vertex);
    unsigned int offset = 0;

    context->VSSetShader(phong_shader->vertex_shader.Get(), 0, 0);
    context->VSSetConstantBuffers(
        0, 1, phong_shader_source->vertex_constant_buffer.GetAddressOf());

    context->PSSetSamplers(0, 1, &phong_shader->sample_state);
    context->PSSetConstantBuffers(
        0, 1, phong_shader_source->pixel_constant_buffer.GetAddressOf());
    context->PSSetShader(phong_shader->pixel_shader.Get(), NULL, 0);
    context->IASetInputLayout(phong_shader->layout.Get());

    for (const auto &mesh : model->meshes) {
        context->PSSetShaderResources(0, 1,
                                      mesh->textureResourceView.GetAddressOf());

        context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(),
                                    &stride, &offset);
        context->IASetIndexBuffer(mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT,
                                  0);

        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->DrawIndexed(model->GetIndexCount(), 0, 0);
    }

    if (gui->SelectedId() == target_id)
        gui->PushNode(phong_shader_source);

    return EnumBehaviorTreeStatus::eSuccess;
}
