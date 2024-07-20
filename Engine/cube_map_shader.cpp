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

    auto mesh_data = GeometryGenerator::MakeBox(40.0f);
    auto cube_map = std::make_shared<Model>(GraphicsManager::Instance().device,
        GraphicsManager::Instance().device_context, std::vector{mesh_data});

    manager->skybox = cube_map;

    auto envFilename = L"./Assets/Textures/Cubemaps/HDRI/SampleEnvHDR.dds";
    auto specularFilename =
        L"./Assets/Textures/Cubemaps/HDRI/SampleSpecularHDR.dds";
    auto irradianceFilename =
        L"./Assets/Textures/Cubemaps/HDRI/SampleDiffuseHDR.dds";
    auto brdfFilename = L"./Assets/Textures/Cubemaps/HDRI/SampleBrdf.dds";

    GraphicsUtil::CreateDDSTexture(GraphicsManager::Instance().device,
                                   envFilename, true,
                                   manager->m_envSRV);
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
        GraphicsManager::Instance().device, L"cube_mapping_vertex_shader.hlsl",
        basicInputElements, cube_map_shader->vertex_shader,
        cube_map_shader->layout);

    GraphicsUtil::CreatePixelShader(GraphicsManager::Instance().device,
                                    L"cube_mapping_pixel_shader.hlsl",
                                    cube_map_shader->pixel_shader);

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

    auto cube_map = dynamic_cast<Model *>(manager->skybox.get());
    auto cube_map_shader = manager->shaders[EnumShaderType::eCube];
    auto cube_shader_source = dynamic_cast<CubeMapShaderSource *>(
        cube_map_shader->source[cube_map->GetEntityId()].get());

    cube_shader_source->pixel_constant.textureToDraw =
        gui->Tab().cube_map.textureToDraw;

    {
        cube_shader_source->vertex_constant.view =
            manager->camera->GetView().Transpose();

        auto env = common::Env::Instance();
        cube_shader_source->vertex_constant.projection =
            manager->camera->GetProjection();

        cube_shader_source->vertex_constant.projection =
            cube_shader_source->vertex_constant.projection.Transpose();
    }

    GraphicsUtil::UpdateBuffer(GraphicsManager::Instance().device,
                               GraphicsManager::Instance().device_context,
                               cube_shader_source->vertex_constant,
                               cube_shader_source->vertex_constant_buffer);

    GraphicsUtil::UpdateBuffer(GraphicsManager::Instance().device,
                               GraphicsManager::Instance().device_context,
                               cube_shader_source->pixel_constant,
                               cube_shader_source->pixel_constant_buffer);

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
    auto cube_map_shader = manager->shaders[EnumShaderType::eCube];
    auto cube_shader_source = dynamic_cast<CubeMapShaderSource *>(
        cube_map_shader->source[cube_map->GetEntityId()].get());

    unsigned int stride = sizeof(Vertex);
    unsigned int offset = 0;

    // RS: Rasterizer stage
    // OM: Output-Merger stage
    // VS: Vertex Shader
    // PS: Pixel Shader
    // IA: Input-Assembler stage
    context->IASetInputLayout(cube_map_shader->layout.Get());
    context->IASetVertexBuffers(
        0, 1, cube_map->meshes.front()->vertexBuffer.GetAddressOf(), &stride,
        &offset);
    context->IASetIndexBuffer(cube_map->meshes.front()->indexBuffer.Get(),
                              DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->VSSetShader(cube_map_shader->vertex_shader.Get(), 0, 0);
    context->VSSetConstantBuffers(
        0, 1, cube_shader_source->vertex_constant_buffer.GetAddressOf());

    std::vector<ID3D11ShaderResourceView *> srvs = {
        manager->m_envSRV.Get(), manager->m_specularSRV.Get(),
        manager->m_irradianceSRV.Get()};
    context->PSSetShaderResources(0, UINT(srvs.size()), srvs.data());

    context->PSSetShader(cube_map_shader->pixel_shader.Get(), 0, 0);
    context->PSSetSamplers(0, 1, cube_map_shader->sample_state.GetAddressOf());

    context->DrawIndexed(cube_map->meshes.front()->indexCount, 0, 0);

    return EnumBehaviorTreeStatus::eSuccess;
}
} // namespace engine