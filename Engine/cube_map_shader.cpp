#include "cube_map_shader.h"
#include "geometry_generator.h"
#include "pipeline_manager.h"
#include "setting_ui.h"

namespace engine {

void CubeMapShaderSource::InitializeThis() {
    vertex_constant.model = Matrix();
    vertex_constant.view = Matrix();
    vertex_constant.projection = Matrix();

    GraphicsUtil::CreateConstBuffer(GraphicsManager::Instance().device,
                                    vertex_constant, vertex_constant_buffer);
    GraphicsUtil::CreateConstBuffer(GraphicsManager::Instance().device,
                                    pixel_constant, pixel_constant_buffer);
}

EnumBehaviorTreeStatus InitializeCubeMapShader::OnInvoke() {
    auto block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<engine::PipelineManager *>(block);
    assert(manager != nullptr);

    auto cube_map_shader = std::make_shared<CubeMapShader>();
    manager->shaders[EnumShaderType::eCube] = cube_map_shader;

    auto mesh_data = GeometryGenerator::MakeBox(10.0f);
    auto cube_map = std::make_shared<Model>(
        GraphicsManager::Instance().device,
        GraphicsManager::Instance().device_context, std::vector{mesh_data});

    manager->skybox = cube_map;

    auto envFilename = L"./Assets/Textures/Cubemaps/HDRI/SampleEnvHDR.dds";
    auto specularFilename =
        L"./Assets/Textures/Cubemaps/HDRI/SampleSpecularHDR.dds";
    auto irradianceFilename =
        L"./Assets/Textures/Cubemaps/HDRI/SampleDiffuseHDR.dds";
    auto brdfFilename = L"./Assets/Textures/Cubemaps/HDRI/SampleBrdf.dds";

    GraphicsUtil::CreateDDSTexture(GraphicsManager::Instance().device,
                                   envFilename, true, manager->m_envSRV);
    GraphicsUtil::CreateDDSTexture(GraphicsManager::Instance().device,
                                   specularFilename, true,
                                   manager->m_specularSRV);
    GraphicsUtil::CreateDDSTexture(GraphicsManager::Instance().device,
                                   irradianceFilename, true,
                                   manager->m_irradianceSRV);
    GraphicsUtil::CreateDDSTexture(GraphicsManager::Instance().device,
                                   brdfFilename, true, manager->m_brdfSRV);

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

    GraphicsUtil::CreateVertexShaderAndInputLayout(
        GraphicsManager::Instance().device, L"SkyboxVS.hlsl",
        basicInputElements, cube_map_shader->vertex_shader,
        cube_map_shader->layout);

    GraphicsUtil::CreatePixelShader(GraphicsManager::Instance().device,
                                    L"SkyboxPS.hlsl",
                                    cube_map_shader->pixel_shader);

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
        &sampDesc, cube_map_shader->sample_state.GetAddressOf());

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus CheckCubeMapShader::CheckCondition() {
    auto block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<engine::PipelineManager *>(block);
    assert(manager != nullptr);

    auto cube_map = dynamic_cast<Model *>(manager->skybox.get());
    assert(cube_map != nullptr);

    auto shader = manager->shaders[EnumShaderType::eCube];
    shader->source[cube_map->GetEntityId()];

    if (shader->source[cube_map->GetEntityId()] == nullptr) {

        auto source = std::make_shared<CubeMapShaderSource>();
        source->Initialize();
        shader->source[cube_map->GetEntityId()] = source;
    }

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus UpdateCubeMap::OnInvoke() {
    auto managerBlock = data_block[EnumDataBlockType::eManager];
    auto guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto gui = dynamic_cast<common::SettingUi *>(guiBlock);
    assert(gui != nullptr);

    manager->m_globalConstsCPU.eyeWorld = manager->camera->GetPosition();
    manager->m_globalConstsCPU.view = manager->camera->GetView().Transpose();
    manager->m_globalConstsCPU.proj =
        manager->camera->GetProjection().Transpose();
    manager->m_globalConstsCPU.invProj =
        manager->m_globalConstsCPU.proj.Invert().Transpose();
    // manager->m_globalConstsCPU.viewProj = (viewRow * projRow).Transpose();

    GraphicsUtil::UpdateBuffer(GraphicsManager::Instance().device,
                               GraphicsManager::Instance().device_context,
                               manager->m_globalConstsCPU,
                               manager->m_globalConstsGPU);

    auto cube_map = dynamic_cast<Model *>(manager->skybox.get());

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus RenderCubeMap::OnInvoke() {
    auto managerBlock = data_block[EnumDataBlockType::eManager];
    auto guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto gui = dynamic_cast<common::SettingUi *>(guiBlock);
    assert(gui != nullptr);

    auto context = GraphicsManager::Instance().device_context;
    auto cube_map = dynamic_cast<Model *>(manager->skybox.get());

    auto shader = manager->shaders[EnumShaderType::eCube];
    shader->source[cube_map->GetEntityId()];

    // 공용 텍스춰들: "Common.hlsli"에서 register(t10)부터 시작
    std::vector<ID3D11ShaderResourceView *> commonSRVs = {
        manager->m_envSRV.Get(), manager->m_specularSRV.Get(),
        manager->m_irradianceSRV.Get(), manager->m_brdfSRV.Get()};

    context->PSSetShaderResources(10, UINT(commonSRVs.size()),
                                    commonSRVs.data());

    //// RS: Rasterizer stage
    //// OM: Output-Merger stage
    //// VS: Vertex Shader
    //// PS: Pixel Shader
    //// IA: Input-Assembler stage
    GraphicsManager::Instance().SetPipelineState(gui->Tab().common.draw_as_wire_
                                  ? Graphics::skyboxWirePSO
                                  : Graphics::skyboxSolidPSO);
    cube_map->Render(context);

    return EnumBehaviorTreeStatus::eSuccess;
}
} // namespace engine