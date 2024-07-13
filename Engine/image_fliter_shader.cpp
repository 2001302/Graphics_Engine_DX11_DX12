#include "image_fliter_shader.h"
#include "geometry_generator.h"
#include "setting_ui.h"
#include "pipeline_manager.h"

using namespace Engine;


EnumBehaviorTreeStatus InitializeBoardMap::OnInvoke() {
    IDataBlock *block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<Engine::PipelineManager *>(block);
    assert(manager != nullptr);

    auto device = Direct3D::Instance().device();
    auto context = Direct3D::Instance().device_context();

    manager->board_map = std::make_unique<BoardMap>();
    GeometryGenerator::MakeSquare(manager->board_map.get());
    manager->board_map->Initialize(
        device, context,
                          {Direct3D::Instance().resolved_SRV()},
                          {Direct3D::Instance().back_buffer_RTV()},
                          Env::Instance().screen_width, Env::Instance().screen_height, 4);


    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus RenderBoardMap::OnInvoke() {
    IDataBlock *managerBlock = data_block[EnumDataBlockType::eManager];
    IDataBlock *guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<Engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    Direct3D::Instance().device_context()->ResolveSubresource(
        Direct3D::Instance().resolved_buffer().Get(), 0,
        Direct3D::Instance().float_buffer().Get(), 0,
        DXGI_FORMAT_R16G16B16A16_FLOAT);

    auto context = Direct3D::Instance().device_context();
    manager->board_map->Render(context);

    return EnumBehaviorTreeStatus::eSuccess;
}
