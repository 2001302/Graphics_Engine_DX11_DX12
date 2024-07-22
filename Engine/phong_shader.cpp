#include "phong_shader.h"
#include "pipeline_manager.h"
#include "setting_ui.h"

namespace engine {

void PhongShader::PhongConstantBufferData::InitializeThis() {
    // create constant buffer(Phong Shader)
    vertex_constant.model = Matrix();
    vertex_constant.view = Matrix();
    vertex_constant.projection = Matrix();

    GraphicsUtil::CreateConstBuffer(GraphicsManager::Instance().device,
                                    vertex_constant, vertex_constant_buffer);
    GraphicsUtil::CreateConstBuffer(GraphicsManager::Instance().device,
                                    pixel_constant, pixel_constant_buffer);
}

void PhongShader::PhongConstantBufferData::OnShow() {

    ImGui::Checkbox("Use Texture", &pixel_constant.useTexture);
    ImGui::Checkbox("Use BlinnPhong", &pixel_constant.useBlinnPhong);

    ImGui::Text("Material");
    ImGui::SliderFloat("Shininess", &pixel_constant.material.shininess, 0.01f,
                       1.0f);

    ImGui::SliderFloat("Diffuse", &pixel_constant.material.diffuse.x, 0.0f,
                       1.0f);
    pixel_constant.material.diffuse =
        Vector3(pixel_constant.material.diffuse.x);

    ImGui::SliderFloat("Specular", &pixel_constant.material.specular.x, 0.0f,
                       1.0f);
    pixel_constant.material.specular =
        Vector3(pixel_constant.material.specular);
};

EnumBehaviorTreeStatus InitializePhongShader::OnInvoke() {
    auto block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<engine::PipelineManager *>(block);
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
    GraphicsManager::Instance().device->CreateSamplerState(
        &sampDesc, phong_shader->sample_state.GetAddressOf());

    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 3,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    GraphicsUtil::CreateVertexShaderAndInputLayout(
        GraphicsManager::Instance().device, L"phong_vertex_shader.hlsl",
        inputElements, phong_shader->vertex_shader, phong_shader->layout);

    GraphicsUtil::CreatePixelShader(GraphicsManager::Instance().device,
                                    L"phong_pixel_shader.hlsl",
                                    phong_shader->pixel_shader);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus CheckPhongShader::OnInvoke() {
    auto model = dynamic_cast<Model *>(target_object);

    if (model->render_mode == EnumRenderMode::eLight)
        return EnumBehaviorTreeStatus::eSuccess;

    return EnumBehaviorTreeStatus::eFail;
}

EnumBehaviorTreeStatus UpdateGameObjectsUsingPhongShader::OnInvoke() {
    auto managerBlock = data_block[EnumDataBlockType::eManager];
    auto guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto gui = dynamic_cast<common::SettingUi *>(guiBlock);
    assert(gui != nullptr);

    auto context = GraphicsManager::Instance().device_context;

    auto model = dynamic_cast<Model *>(target_object);

    auto phong_shader = manager->shaders[EnumShaderType::ePhong];
    if (phong_shader->source[target_object->GetEntityId()] == nullptr) {

        auto source = std::make_shared<PhongShader::PhongConstantBufferData>();
        source->Initialize();
        phong_shader->source[target_object->GetEntityId()] = source;
    }
    auto phong_shader_source =
        dynamic_cast<PhongShader::PhongConstantBufferData *>(
            phong_shader->source[model->GetEntityId()].get());

    // model
    {
        //phong_shader_source->vertex_constant.model =
        //    Matrix::CreateScale(model->scaling) *
        //    Matrix::CreateRotationX(model->rotation.x) *
        //    Matrix::CreateRotationY(model->rotation.y) *
        //    Matrix::CreateRotationZ(model->rotation.z) *
        //    Matrix::CreateTranslation(model->translation);

        //phong_shader_source->vertex_constant.model =
        //    phong_shader_source->vertex_constant.model.Transpose();
    }
    // view
    //{
    //    phong_shader_source->vertex_constant.view =
    //        manager->camera->GetView().Transpose();
    //}
    // inverse transpose
    {
        phong_shader_source->vertex_constant.invTranspose =
            phong_shader_source->vertex_constant.model;
        phong_shader_source->vertex_constant.invTranspose.Translation(
            Vector3(0.0f));
        phong_shader_source->vertex_constant.invTranspose =
            phong_shader_source->vertex_constant.invTranspose.Transpose()
                .Invert();
    }
    // projection
    //{
    //    auto env = common::Env::Instance();
    //    phong_shader_source->vertex_constant.projection =
    //        manager->camera->GetProjection().Transpose();
    //}

    GraphicsUtil::UpdateBuffer(GraphicsManager::Instance().device,
                               GraphicsManager::Instance().device_context,
                               phong_shader_source->vertex_constant,
                               phong_shader_source->vertex_constant_buffer);

    // eye
    {
        phong_shader_source->pixel_constant.eyeWorld = Vector3::Transform(
            Vector3(0.0f), phong_shader_source->vertex_constant.view.Invert());
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
            phong_shader_source->pixel_constant.lights[i].radiance*= 0.0f;   
        }
    }

    // phong_shader_source->pixel_constant_buffer_data.useTexture =
    //     detail->use_texture;
    // phong_shader_source->pixel_constant_buffer_data.useBlinnPhong =
    //    gui->Tab().light.use_blinn_phong;

    GraphicsUtil::UpdateBuffer(GraphicsManager::Instance().device,
                               GraphicsManager::Instance().device_context,
                               phong_shader_source->pixel_constant,
                               phong_shader_source->pixel_constant_buffer);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus RenderGameObjectsUsingPhongShader::OnInvoke() {
    auto managerBlock = data_block[EnumDataBlockType::eManager];
    auto guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto gui = dynamic_cast<common::SettingUi *>(guiBlock);
    assert(gui != nullptr);

    auto context = GraphicsManager::Instance().device_context;

    // RS: Rasterizer stage
    // OM: Output-Merger stage
    // VS: Vertex Shader
    // PS: Pixel Shader
    // IA: Input-Assembler stage

    auto model = dynamic_cast<Model *>(target_object);
    auto phong_shader = manager->shaders[EnumShaderType::ePhong];
    auto phong_shader_source =
        dynamic_cast<PhongShader::PhongConstantBufferData *>(
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

        std::vector<ID3D11ShaderResourceView *> resViews = {
            mesh->albedoSRV.Get(),
            manager->m_specularSRV.Get(),
            manager->m_irradianceSRV.Get(),
        };

        context->PSSetShaderResources(0, UINT(resViews.size()),
                                      resViews.data());

        context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(),
                                    &stride, &offset);
        context->IASetIndexBuffer(mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT,
                                  0);

        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->DrawIndexed(mesh->indexCount, 0, 0);
    }

    if (gui->SelectedId() == target_object->GetEntityId())
        gui->PushNode(phong_shader_source);

    return EnumBehaviorTreeStatus::eSuccess;
}
} // namespace engine