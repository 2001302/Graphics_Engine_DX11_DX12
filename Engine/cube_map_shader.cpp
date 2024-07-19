#include "cube_map_shader.h"
#include "geometry_generator.h"
#include "pipeline_manager.h"
#include "setting_ui.h"

using namespace dx11;

void CubeMapShaderSource::InitializeThis() {
    vertex_constant.model = Matrix();
    vertex_constant.view = Matrix();
    vertex_constant.projection = Matrix();

    GraphicsContext::Instance().CreateConstantBuffer(
        vertex_constant, vertex_constant_buffer);
    GraphicsContext::Instance().CreateConstantBuffer(pixel_constant,
                                                     pixel_constant_buffer);
}

EnumBehaviorTreeStatus InitializeCubeMapShader::OnInvoke() {
    auto block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<dx11::PipelineManager *>(block);
    assert(manager != nullptr);

    auto cube_map = std::make_shared<CubeMap>();
    manager->cube_map = cube_map;
    GeometryGenerator::MakeBox(cube_map.get());

    auto cube_map_shader = std::make_shared<CubeMapShader>();
    manager->shaders[EnumShaderType::eCube] = cube_map_shader;

    cube_map->texture = std::make_shared<CubeMap::CubeTexture>();

    std::reverse(cube_map->mesh->indices.begin(),
                 cube_map->mesh->indices.end());

    auto envFilename = L"./Assets/Textures/Cubemaps/HDRI/SampleEnvHDR.dds";
    auto specularFilename =
        L"./Assets/Textures/Cubemaps/HDRI/SampleSpecularHDR.dds";
    auto irradianceFilename =
        L"./Assets/Textures/Cubemaps/HDRI/SampleDiffuseHDR.dds";
    auto brdfFilename = L"./Assets/Textures/Cubemaps/HDRI/SampleBrdf.dds";

    GraphicsContext::Instance().CreateDDSTexture(envFilename,
                                                 cube_map->texture->env_SRV);
    GraphicsContext::Instance().CreateDDSTexture(
        specularFilename, cube_map->texture->specular_SRV);
    GraphicsContext::Instance().CreateDDSTexture(
        irradianceFilename, cube_map->texture->irradiance_SRV);
    GraphicsContext::Instance().CreateDDSTexture(brdfFilename,
                                                 cube_map->texture->brdf_SRV);

    GraphicsContext::Instance().CreateVertexBuffer(
        cube_map->mesh->vertices, cube_map->mesh->vertexBuffer);
    GraphicsContext::Instance().CreateIndexBuffer(cube_map->mesh->indices,
                                                  cube_map->mesh->indexBuffer);

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

    GraphicsContext::Instance().CreateVertexShaderAndInputLayout(
        L"cube_mapping_vertex_shader.hlsl", basicInputElements,
        cube_map_shader->vertex_shader, cube_map_shader->layout);

    GraphicsContext::Instance().CreatePixelShader(
        L"cube_mapping_pixel_shader.hlsl", cube_map_shader->pixel_shader);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus CheckCubeMapShader::CheckCondition() {
    auto block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<dx11::PipelineManager *>(block);
    assert(manager != nullptr);

    auto cube_map = dynamic_cast<CubeMap *>(manager->cube_map.get());
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

    auto manager = dynamic_cast<dx11::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto gui = dynamic_cast<common::SettingUi *>(guiBlock);
    assert(gui != nullptr);

    auto cube_map = dynamic_cast<CubeMap *>(manager->cube_map.get());
    auto cube_map_shader = manager->shaders[EnumShaderType::eCube];
    auto cube_shader_source = dynamic_cast<CubeMapShaderSource *>(
        cube_map_shader->source[cube_map->GetEntityId()].get());

    cube_shader_source->pixel_constant.textureToDraw =
        gui->Tab().cube_map.textureToDraw;

    {
        cube_shader_source->vertex_constant.view =
            manager->camera->view.Transpose();

        auto env = common::Env::Instance();
        cube_shader_source->vertex_constant.projection =
            XMMatrixPerspectiveFovLH(
                XMConvertToRadians(env.projection.projection_fov_angle_y),
                env.aspect, env.projection.near_z, env.projection.far_z);

        cube_shader_source->vertex_constant.projection =
            cube_shader_source->vertex_constant.projection
                .Transpose();
    }

    GraphicsContext::Instance().UpdateBuffer(
        cube_shader_source->vertex_constant,
        cube_shader_source->vertex_constant_buffer);

    GraphicsContext::Instance().UpdateBuffer(
        cube_shader_source->pixel_constant,
        cube_shader_source->pixel_constant_buffer);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus RenderCubeMap::OnInvoke() {
    auto managerBlock = data_block[EnumDataBlockType::eManager];
    auto guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<dx11::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto gui = dynamic_cast<common::SettingUi *>(guiBlock);
    assert(gui != nullptr);

    auto context = GraphicsContext::Instance().device_context();
    auto cube_map = dynamic_cast<CubeMap *>(manager->cube_map.get());
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
        0, 1, cube_map->mesh->vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(cube_map->mesh->indexBuffer.Get(),
                              DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->VSSetShader(cube_map_shader->vertex_shader.Get(), 0, 0);
    context->VSSetConstantBuffers(
        0, 1, cube_shader_source->vertex_constant_buffer.GetAddressOf());

    std::vector<ID3D11ShaderResourceView *> srvs = {
        cube_map->texture->env_SRV.Get(), cube_map->texture->specular_SRV.Get(),
        cube_map->texture->irradiance_SRV.Get()};
    context->PSSetShaderResources(0, UINT(srvs.size()), srvs.data());

    context->PSSetShader(cube_map_shader->pixel_shader.Get(), 0, 0);
    context->PSSetSamplers(0, 1, cube_map_shader->sample_state.GetAddressOf());

    context->DrawIndexed(cube_map->GetIndexCount(), 0, 0);

    return EnumBehaviorTreeStatus::eSuccess;
}
