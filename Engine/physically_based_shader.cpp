#include "physically_based_shader.h"
#include "geometry_generator.h"
#include "setting_ui.h"
#include "pipeline_manager.h"

using namespace dx11;

void PhsicallyBasedShaderSource::InitializeThis() {
    vertex_constant_buffer_data.modelWorld = Matrix();
    vertex_constant_buffer_data.view = Matrix();
    vertex_constant_buffer_data.projection = Matrix();
    vertex_constant_buffer_data.useHeightMap = 0;
    vertex_constant_buffer_data.heightScale = 0.0f;

    Direct3D::Instance().CreateConstantBuffer(vertex_constant_buffer_data,
                                                 vertex_constant_buffer);
    Direct3D::Instance().CreateConstantBuffer(pixel_constant_buffer_data,
                                                 pixel_constant_buffer);
}

EnumBehaviorTreeStatus CheckPhysicallyBasedShader::CheckCondition() {
    auto guiBlock = data_block[EnumDataBlockType::eGui];
    auto gui = dynamic_cast<common::SettingUi *>(guiBlock);
    assert(gui != nullptr);

    if (gui->GetGlobalTab().render_mode ==
        common::EnumRenderMode::ePhysicallyBasedRendering) {
        return EnumBehaviorTreeStatus::eSuccess;
    }

    return EnumBehaviorTreeStatus::eFail;
}

EnumBehaviorTreeStatus InitializePhysicallyBasedShader::OnInvoke() {
    auto block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<dx11::PipelineManager *>(block);
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
    Direct3D::Instance().device()->CreateSamplerState(
        &sampDesc, physical_shader->sample_state.GetAddressOf());

    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

    Direct3D::Instance().device()->CreateSamplerState(
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

    Direct3D::Instance().CreateVertexShaderAndInputLayout(
        L"physically_based_vertex_shader.hlsl", inputElements,
        physical_shader->vertex_shader, physical_shader->layout);

    Direct3D::Instance().CreatePixelShader(
        L"physically_based_pixel_shader.hlsl", physical_shader->pixel_shader);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus UpdateGameObjectsUsingPhysicallyBasedShader::OnInvoke() {
    auto managerBlock = data_block[EnumDataBlockType::eManager];
    auto guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<dx11::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto gui = dynamic_cast<common::SettingUi *>(guiBlock);
    assert(gui != nullptr);

    auto context = Direct3D::Instance().device_context();

    auto model = dynamic_cast<Model *>(manager->models[target_id].get());

    auto physically_shader = manager->shaders[EnumShaderType::ePhysicallyBased];
    if (physically_shader->source[target_id] == nullptr) {

        auto source = std::make_shared<PhsicallyBasedShaderSource>();
        source->Initialize();
        physically_shader->source[target_id] = source;
    }
    auto physically_shader_source = dynamic_cast<PhsicallyBasedShaderSource *>(
        physically_shader->source[model->GetEntityId()].get());

    // model
    {
        physically_shader_source->vertex_constant_buffer_data.modelWorld =
            Matrix::CreateScale(model->scaling) *
            Matrix::CreateRotationX(model->rotation.x) *
            Matrix::CreateRotationY(model->rotation.y) *
            Matrix::CreateRotationZ(model->rotation.z) *
            Matrix::CreateTranslation(model->translation);

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
            physically_shader_source->vertex_constant_buffer_data.modelWorld;
        physically_shader_source->vertex_constant_buffer_data.invTranspose
            .Translation(Vector3(0.0f));
        physically_shader_source->vertex_constant_buffer_data.invTranspose =
            physically_shader_source->vertex_constant_buffer_data.invTranspose
                .Transpose()
                .Invert();
    }
    // projection
    {
        const float aspect = common::Env::Instance().aspect;

        physically_shader_source->vertex_constant_buffer_data
            .projection = XMMatrixPerspectiveFovLH(
            XMConvertToRadians(
                gui->GetGlobalTab().projection_setting.projection_fov_angle_y),
            aspect, gui->GetGlobalTab().projection_setting.near_z,
            gui->GetGlobalTab().projection_setting.far_z);

        physically_shader_source->vertex_constant_buffer_data.projection =
            physically_shader_source->vertex_constant_buffer_data.projection
                .Transpose();
    }

    Direct3D::Instance().UpdateBuffer(
        physically_shader_source->vertex_constant_buffer_data,
        physically_shader_source->vertex_constant_buffer);

    // eye
    {
        physically_shader_source->pixel_constant_buffer_data.eyeWorld =
            Vector3::Transform(Vector3(0.0f),
                               physically_shader_source
                                   ->vertex_constant_buffer_data.view.Invert());
    }
    // material
    {
        physically_shader_source->pixel_constant_buffer_data.material
            .roughness = gui->GetGlobalTab().pbr_setting.roughness;
        physically_shader_source->pixel_constant_buffer_data.material.metallic =
            gui->GetGlobalTab().pbr_setting.metallic;
    }
    // light
    {
        for (int i = 0; i < MAX_LIGHTS; i++) {
            if (i != gui->GetGlobalTab().light_setting.light_type) {
                physically_shader_source->pixel_constant_buffer_data.lights[i]
                    .strength *= 0.0f;
            } else {
                // turn off another light
                physically_shader_source->pixel_constant_buffer_data.lights[i] =
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

    Direct3D::Instance().UpdateBuffer(
        physically_shader_source->pixel_constant_buffer_data,
        physically_shader_source->pixel_constant_buffer);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus RenderGameObjectsUsingPhysicallyBasedShader::OnInvoke() {
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
    auto physically_shader = std::static_pointer_cast<PhsicallyBasedShader>(
        manager->shaders[EnumShaderType::ePhysicallyBased]);
    auto physically_shader_source = dynamic_cast<PhsicallyBasedShaderSource *>(
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
            manager->cube_map->texture->specular_SRV.Get(),
            manager->cube_map->texture->irradiance_SRV.Get(),
            manager->cube_map->texture->brdf_SRV.Get(),
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
        context->IASetIndexBuffer(mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT,
                                  0);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        context->DrawIndexed(model->GetIndexCount(), 0, 0);
    }

    return EnumBehaviorTreeStatus::eSuccess;
}
