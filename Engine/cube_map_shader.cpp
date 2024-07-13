#include "cube_map_shader.h"
#include "geometry_generator.h"
#include "pipeline_manager.h"
#include "setting_ui.h"

using namespace dx11;

void CubeMapShaderSource::InitializeThis() {
    vertex_constant_buffer_data.model = Matrix();
    vertex_constant_buffer_data.view = Matrix();
    vertex_constant_buffer_data.projection = Matrix();

    Direct3D::Instance().CreateConstantBuffer(vertex_constant_buffer_data,
                                                 vertex_constant_buffer);
    Direct3D::Instance().CreateConstantBuffer(pixel_constant_buffer_data,
                                                 pixel_constant_buffer);
}

EnumBehaviorTreeStatus InitializeCubeMapShader::OnInvoke() {
    IDataBlock *block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<dx11::PipelineManager *>(block);
    assert(manager != nullptr);

    manager->cube_map = std::make_shared<CubeMap>();
    GeometryGenerator::MakeBox(manager->cube_map.get());

    auto cube_map_shader = std::make_shared<CubeMapShader>();
    manager->shaders[EnumShaderType::eCube] = cube_map_shader;

    manager->cube_map->texture = std::make_shared<CubeMap::CubeTexture>();

    std::reverse(manager->cube_map->mesh->indices.begin(),
                 manager->cube_map->mesh->indices.end());

    auto envFilename = L"./Assets/Textures/Cubemaps/HDRI/SampleEnvHDR.dds";
    auto specularFilename =
        L"./Assets/Textures/Cubemaps/HDRI/SampleSpecularHDR.dds";
    auto irradianceFilename =
        L"./Assets/Textures/Cubemaps/HDRI/SampleDiffuseHDR.dds";
    auto brdfFilename = L"./Assets/Textures/Cubemaps/HDRI/SampleBrdf.dds";

    Direct3D::Instance().CreateDDSTexture(
        envFilename, manager->cube_map->texture->env_SRV);
    Direct3D::Instance().CreateDDSTexture(
        specularFilename, manager->cube_map->texture->specular_SRV);
    Direct3D::Instance().CreateDDSTexture(
        irradianceFilename, manager->cube_map->texture->irradiance_SRV);
    Direct3D::Instance().CreateDDSTexture(
        brdfFilename, manager->cube_map->texture->brdf_SRV);

    Direct3D::Instance().CreateVertexBuffer(
        manager->cube_map->mesh->vertices,
        manager->cube_map->mesh->vertexBuffer);
    Direct3D::Instance().CreateIndexBuffer(
        manager->cube_map->mesh->indices, manager->cube_map->mesh->indexBuffer);

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

    Direct3D::Instance().CreateVertexShaderAndInputLayout(
        L"cube_mapping_vertex_shader.hlsl", basicInputElements,
        cube_map_shader->vertex_shader, cube_map_shader->layout);

    Direct3D::Instance().CreatePixelShader(L"cube_mapping_pixel_shader.hlsl",
                                              cube_map_shader->pixel_shader);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus CheckCubeMapShader::CheckCondition() {
    IDataBlock *block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<dx11::PipelineManager *>(block);
    assert(manager != nullptr);

    auto shader = manager->shaders[EnumShaderType::eCube];
    shader->source[manager->cube_map->GetEntityId()];

    if (shader->source[manager->cube_map->GetEntityId()] == nullptr) {

        auto source = std::make_shared<CubeMapShaderSource>();
        source->Initialize();
        shader->source[manager->cube_map->GetEntityId()] = source;
    }

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus UpdateCubeMap::OnInvoke() {
    IDataBlock *managerBlock = data_block[EnumDataBlockType::eManager];
    IDataBlock *guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<dx11::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto gui = dynamic_cast<dx11::SettingUi *>(guiBlock);
    assert(gui != nullptr);

    auto cube_map = manager->cube_map;
    auto cube_map_shader = manager->shaders[EnumShaderType::eCube];
    auto cube_shader_source = dynamic_cast<CubeMapShaderSource *>(
        cube_map_shader->source[cube_map->GetEntityId()].get());

    cube_shader_source->pixel_constant_buffer_data.textureToDraw =
        gui->GetGlobalTab().cube_map_setting.textureToDraw;

    {
        cube_shader_source->vertex_constant_buffer_data.view =
            manager->camera->view.Transpose();

        const float aspect = Env::Instance().aspect;
        cube_shader_source->vertex_constant_buffer_data
            .projection = XMMatrixPerspectiveFovLH(
            XMConvertToRadians(
                gui->GetGlobalTab().projection_setting.projection_fov_angle_y),
            aspect, gui->GetGlobalTab().projection_setting.near_z,
            gui->GetGlobalTab().projection_setting.far_z);

        cube_shader_source->vertex_constant_buffer_data.projection =
            cube_shader_source->vertex_constant_buffer_data.projection
                .Transpose();
    }

    Direct3D::Instance().UpdateBuffer(
        cube_shader_source->vertex_constant_buffer_data,
        cube_shader_source->vertex_constant_buffer);

    Direct3D::Instance().UpdateBuffer(
        cube_shader_source->pixel_constant_buffer_data,
        cube_shader_source->pixel_constant_buffer);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus RenderCubeMap::OnInvoke() {
    IDataBlock *managerBlock = data_block[EnumDataBlockType::eManager];
    IDataBlock *guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<dx11::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    auto gui = dynamic_cast<dx11::SettingUi *>(guiBlock);
    assert(gui != nullptr);

    auto context = Direct3D::Instance().device_context();
    auto cube_map = manager->cube_map;
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
