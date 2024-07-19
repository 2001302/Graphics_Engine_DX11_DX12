#include "image_fliter_shader.h"
#include "geometry_generator.h"
#include "pipeline_manager.h"
#include "setting_ui.h"

using namespace dx11;

EnumBehaviorTreeStatus InitializeBoardMap::OnInvoke() {
    auto block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<dx11::PipelineManager *>(block);
    assert(manager != nullptr);

    auto device = GraphicsManager::Instance().device;
    auto context = GraphicsManager::Instance().device_context;

    manager->board_map = std::make_unique<BoardMap>();
    GeometryGenerator::MakeSquare(manager->board_map.get());
    manager->board_map->Initialize(
        device, context, {GraphicsManager::Instance().resolved_SRV},
        {GraphicsManager::Instance().back_buffer_RTV},
        common::Env::Instance().screen_width,
        common::Env::Instance().screen_height, 4);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus RenderBoardMap::OnInvoke() {
    auto managerBlock = data_block[EnumDataBlockType::eManager];
    auto guiBlock = data_block[EnumDataBlockType::eGui];

    auto manager = dynamic_cast<dx11::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    GraphicsManager::Instance().device_context->ResolveSubresource(
        GraphicsManager::Instance().resolved_buffer.Get(), 0,
        GraphicsManager::Instance().float_buffer.Get(), 0,
        DXGI_FORMAT_R16G16B16A16_FLOAT);

    auto context = GraphicsManager::Instance().device_context;
    manager->board_map->Render(context);

    return EnumBehaviorTreeStatus::eSuccess;
}
