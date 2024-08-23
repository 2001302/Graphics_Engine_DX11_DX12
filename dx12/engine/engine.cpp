#include "engine.h"
#include "../foundation/behavior_tree_builder.h"

namespace core {

auto camera_node = std::make_shared<CameraNodeInvoker>();
auto shared_resource_node = std::make_shared<SharedResourceNodeInvoker>();
auto game_object_node = std::make_shared<GameObjectNodeInvoker>();
auto gui_node = std::make_shared<GuiNodeInvoker>();
auto command_manage_node = std::make_shared<CommandManageNode>();
auto present_node = std::make_shared<PresentNode>();
auto light_node = std::make_shared<LightNodeInvoker>();

Engine::Engine() {
    black_board = std::make_shared<BlackBoard>();
    message_receiver = std::make_unique<MessageReceiver>();
    start_tree = std::make_shared<foundation::BehaviorTreeBuilder>();
    update_tree = std::make_shared<foundation::BehaviorTreeBuilder>();
    render_tree = std::make_shared<foundation::BehaviorTreeBuilder>();
};

bool Engine::Start() {
    // clang-format off
    Platform::Start();

    dx12::GpuCore::Instance().Initialize();
    
    black_board->job_context->stage_type = EnumStageType::eInitialize;
    
    //initialize
    start_tree->Build(black_board.get())
    ->Sequence()
        ->Excute(command_manage_node)
        ->Excute(camera_node)
        ->Excute(light_node)
        ->Excute(shared_resource_node)
        ->Excute(game_object_node)
        ->Excute(gui_node)
    ->Close()
    ->Run();
    
    //update
    update_tree->Build(black_board.get())
    ->Sequence()
        ->Excute(camera_node)
        ->Excute(light_node)
        ->Excute(shared_resource_node)
        ->Excute(game_object_node)
    ->Close();
     
    //render
    render_tree->Build(black_board.get())
    ->Sequence()
        ->Excute(command_manage_node)
        ->Excute(game_object_node)
        ->Excute(gui_node)
        ->Excute(present_node)
    ->Close();

    Frame();

    // clang-format on
    return true;
}

bool Engine::Frame() {

    // update
    black_board->job_context->dt = ImGui::GetIO().DeltaTime;
    black_board->job_context->stage_type = EnumStageType::eUpdate;
    update_tree->Run();

    // render
    black_board->job_context->stage_type = EnumStageType::eRender;
    render_tree->Run();

    return true;
}

bool Engine::Stop() {
    Platform::Stop();

    if (black_board) {
        if (black_board->job_context) {
            for (auto &model : black_board->job_context->objects) {
                model.second.reset();
            }
            black_board->job_context->camera.reset();
        }
        if (black_board->gui) {
            black_board->gui->Shutdown();
            black_board->gui.reset();
        }
        if (black_board->input) {
            black_board->input.reset();
        }
    }

    return true;
}

} // namespace core
