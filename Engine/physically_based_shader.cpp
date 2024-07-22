#include "physically_based_shader.h"
#include "geometry_generator.h"
#include "pipeline_manager.h"
#include "setting_ui.h"

namespace engine {

void PhsicallyBasedShader::PhsicallyBasedConstantBufferData::InitializeThis() {
    vertex_constant.modelWorld = Matrix();
    vertex_constant.view = Matrix();
    vertex_constant.projection = Matrix();
    vertex_constant.useHeightMap = 0;
    vertex_constant.heightScale = 0.0f;

    GraphicsUtil::CreateConstBuffer(GraphicsManager::Instance().device,
                                    vertex_constant, vertex_constant_buffer);
    GraphicsUtil::CreateConstBuffer(GraphicsManager::Instance().device,
                                    pixel_constant, pixel_constant_buffer);
}

void IntCheckbox(const char *label, int *v) {
    bool b = (*v != 0);
    if (ImGui::Checkbox(label, &b)) {
        *v = b ? 1 : 0;
    }
}

void PhsicallyBasedShader::PhsicallyBasedConstantBufferData::OnShow() {

    // float mipmapLevel = 0.0f; // 4
    // float expose = 1.0f;      // 16
    // float gamma = 1.0f;

    IntCheckbox("Use Texture", &pixel_constant.useAlbedoMap);

    bool useNormalMap = (bool)pixel_constant.useNormalMap;
    ImGui::Checkbox("useNormalMap", &useNormalMap);
    pixel_constant.useNormalMap = useNormalMap;

    bool useAOMap = (bool)pixel_constant.useAOMap;
    ImGui::Checkbox("useAOMap", &useAOMap);
    pixel_constant.useAOMap = useAOMap;

    bool invertNormalMapY = (bool)pixel_constant.invertNormalMapY;
    ImGui::Checkbox("invertNormalMapY", &invertNormalMapY);
    pixel_constant.invertNormalMapY = invertNormalMapY;

    bool useMetallicMap = (bool)pixel_constant.useMetallicMap;
    ImGui::Checkbox("useMetallicMap", &useMetallicMap);
    pixel_constant.useMetallicMap = useMetallicMap;

    bool useRoughnessMap = (bool)pixel_constant.useRoughnessMap;
    ImGui::Checkbox("useRoughnessMap", &useRoughnessMap);
    pixel_constant.useRoughnessMap = useRoughnessMap;

    bool useEmissiveMap = (bool)pixel_constant.useEmissiveMap;
    ImGui::Checkbox("useEmissiveMap", &useEmissiveMap);
    pixel_constant.useRoughnessMap = useEmissiveMap;

    ImGui::Text("Material");

    ImGui::SliderFloat("albedo", &pixel_constant.material.albedo.x, 0.01f,
                       1.0f);
    pixel_constant.material.albedo = Vector3(pixel_constant.material.albedo.x);

    ImGui::SliderFloat("roughness", &pixel_constant.material.roughness, 0.0f,
                       1.0f);

    ImGui::SliderFloat("metallic", &pixel_constant.material.metallic, 0.0f,
                       1.0f);
};

EnumBehaviorTreeStatus CheckPhysicallyBasedShader::OnInvoke() {

    auto model = dynamic_cast<Model *>(target_object);

    if (model->render_mode == EnumRenderMode::ePhysicallyBasedRendering)
        return EnumBehaviorTreeStatus::eSuccess;

    return EnumBehaviorTreeStatus::eFail;
}

EnumBehaviorTreeStatus InitializePhysicallyBasedShader::OnInvoke() {
    auto block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<engine::PipelineManager *>(block);
    assert(manager != nullptr);

    // Create and initialize the light shader object.
    auto physical_shader = std::make_shared<PhsicallyBasedShader>();
    manager->shaders[EnumShaderType::ePhysicallyBased] = physical_shader;

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
        &sampDesc, physical_shader->sample_state.GetAddressOf());

    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

    GraphicsManager::Instance().device->CreateSamplerState(
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

    GraphicsUtil::CreateVertexShaderAndInputLayout(
        GraphicsManager::Instance().device,
        L"physically_based_vertex_shader.hlsl", inputElements,
        physical_shader->vertex_shader, physical_shader->layout);

    GraphicsUtil::CreatePixelShader(GraphicsManager::Instance().device,
                                    L"physically_based_pixel_shader.hlsl",
                                    physical_shader->pixel_shader);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus UpdateGameObjectsUsingPhysicallyBasedShader::OnInvoke() {
    auto managerBlock = data_block[EnumDataBlockType::eManager];
    auto guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto gui = dynamic_cast<common::SettingUi *>(guiBlock);
    assert(gui != nullptr);

    auto context = GraphicsManager::Instance().device_context;

    auto model = dynamic_cast<Model *>(target_object);

    auto physically_shader = manager->shaders[EnumShaderType::ePhysicallyBased];
    if (physically_shader->source[target_object->GetEntityId()] == nullptr) {

        auto source = std::make_shared<
            PhsicallyBasedShader::PhsicallyBasedConstantBufferData>();
        source->Initialize();
        physically_shader->source[target_object->GetEntityId()] = source;
    }
    auto physically_shader_source =
        dynamic_cast<PhsicallyBasedShader::PhsicallyBasedConstantBufferData *>(
            physically_shader->source[model->GetEntityId()].get());

    // model
    {
        //physically_shader_source->vertex_constant.modelWorld =
        //    Matrix::CreateScale(model->scaling) *
        //    Matrix::CreateRotationX(model->rotation.x) *
        //    Matrix::CreateRotationY(model->rotation.y) *
        //    Matrix::CreateRotationZ(model->rotation.z) *
        //    Matrix::CreateTranslation(model->translation);

        //physically_shader_source->vertex_constant.modelWorld =
        //    physically_shader_source->vertex_constant.modelWorld.Transpose();
    }
    // view
    //{
    //    physically_shader_source->vertex_constant.view =
    //        manager->camera->GetView().Transpose();
    //}
    // inverse transpose
    {
        physically_shader_source->vertex_constant.invTranspose =
            physically_shader_source->vertex_constant.modelWorld;
        physically_shader_source->vertex_constant.invTranspose.Translation(
            Vector3(0.0f));
        physically_shader_source->vertex_constant.invTranspose =
            physically_shader_source->vertex_constant.invTranspose.Transpose()
                .Invert();
    }
    // projection
    //{
    //    auto env = common::Env::Instance();
    //    physically_shader_source->vertex_constant.projection =
    //        manager->camera->GetProjection().Transpose();
    //}

    GraphicsUtil::UpdateBuffer(
        GraphicsManager::Instance().device,
        GraphicsManager::Instance().device_context,
        physically_shader_source->vertex_constant,
        physically_shader_source->vertex_constant_buffer);

    // eye
    {
        physically_shader_source->pixel_constant.eyeWorld = Vector3::Transform(
            Vector3(0.0f),
            physically_shader_source->vertex_constant.view.Invert());
    }
    //// material
    //{
    //    physically_shader_source->pixel_constant_buffer_data.material
    //        .roughness = gui->Tab().pbr.roughness;
    //    physically_shader_source->pixel_constant_buffer_data.material.metallic
    //    =
    //        gui->Tab().pbr.metallic;
    //}
    // light
    //{
    //    for (int i = 0; i < MAX_LIGHTS; i++) {
    //        if (i != gui->Tab().light.light_type) {
    //            physically_shader_source->pixel_constant.lights[i].strength *=
    //                0.0f;
    //        } else {
    //            // turn off another light
    //            physically_shader_source->pixel_constant.lights[i] =
    //                gui->Tab().light.light_from_gui;
    //        }
    //    }
    //}

    GraphicsUtil::UpdateBuffer(GraphicsManager::Instance().device,
                               GraphicsManager::Instance().device_context,
                               physically_shader_source->pixel_constant,
                               physically_shader_source->pixel_constant_buffer);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus RenderGameObjectsUsingPhysicallyBasedShader::OnInvoke() {
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
    auto physically_shader = std::static_pointer_cast<PhsicallyBasedShader>(
        manager->shaders[EnumShaderType::ePhysicallyBased]);

    auto physically_shader_source =
        dynamic_cast<PhsicallyBasedShader::PhsicallyBasedConstantBufferData *>(
            physically_shader->source[model->GetEntityId()].get());
    assert(physically_shader != nullptr);

    unsigned int stride = sizeof(Vertex);
    unsigned int offset = 0;

    for (const auto &mesh : model->meshes) {

        // VertexShader에서도 Texture 사용
        context->VSSetShader(physically_shader->vertex_shader.Get(), 0, 0);
        context->VSSetShaderResources(0, 1, mesh->heightSRV.GetAddressOf());
        context->VSSetSamplers(0, 1,
                               physically_shader->sample_state.GetAddressOf());
        context->VSSetConstantBuffers(
            0, 1,
            physically_shader_source->vertex_constant_buffer.GetAddressOf());

        std::vector<ID3D11SamplerState *> samplers = {
            physically_shader->sample_state.Get(),
            physically_shader->clampSamplerState.Get()};

        context->PSSetSamplers(0, UINT(samplers.size()), samplers.data());
        context->PSSetShader(physically_shader->pixel_shader.Get(), 0, 0);

        std::vector<ID3D11ShaderResourceView *> resViews = {
            manager->m_specularSRV.Get(),
            manager->m_irradianceSRV.Get(),
            manager->m_brdfSRV.Get(),
            mesh->albedoSRV.Get(),
            mesh->normalSRV.Get(),
            mesh->aoSRV.Get(),
            mesh->metallicRoughnessSRV.Get(),
            nullptr,
            mesh->emissiveSRV.Get()};
        context->PSSetShaderResources(0, UINT(resViews.size()),
                                      resViews.data());

        context->PSSetConstantBuffers(
            0, 1,
            physically_shader_source->pixel_constant_buffer.GetAddressOf());

        context->IASetInputLayout(physically_shader->layout.Get());
        context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(),
                                    &stride, &offset);
        context->IASetIndexBuffer(mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT,
                                  0);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        context->DrawIndexed(mesh->indexCount, 0, 0);
    }

    if (gui->SelectedId() == target_object->GetEntityId())
        gui->PushNode(physically_shader_source);

    return EnumBehaviorTreeStatus::eSuccess;
}
} // namespace engine