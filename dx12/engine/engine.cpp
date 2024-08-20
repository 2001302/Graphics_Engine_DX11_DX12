#include "engine.h"
#include "../foundation/behavior_tree_builder.h"

namespace core {

Engine::Engine() {
    black_board = std::make_shared<BlackBoard>();
    message_receiver = std::make_unique<MessageReceiver>();
    gui = std::make_shared<foundation::SettingUi>();
    start_tree = std::make_shared<foundation::BehaviorTreeBuilder>();
    update_tree = std::make_shared<foundation::BehaviorTreeBuilder>();
    render_tree = std::make_shared<foundation::BehaviorTreeBuilder>();

    
    camera_node = std::make_shared<CameraNodeInvoker>();
    light_node = std::make_shared<LightNodeInvoker>();
    shared_resource_node = std::make_shared<SharedResourceNodeInvoker>();
    game_object_node = std::make_shared<GameObjectNodeInvoker>();
    gui_node = std::make_shared<GuiNodeInvoker>();
    start_rendering_node = std::make_shared<StartRenderingNode>();
    present_node = std::make_shared<PresentNode>();
};

bool Engine::Start() {
    // clang-format off
    Platform::Start();

    dx12::GpuCore::Instance().Initialize();
    gui->Start();
    black_board->job_context->stage_type = EnumStageType::eInitialize;
    
    //initialize
    start_tree->Build(black_board.get())
    ->Sequence()
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
        ->Excute(start_rendering_node)
        ->Excute(game_object_node)
        ->Excute(gui_node)
        ->Excute(present_node)
    ->Close();

    Frame();

    // clang-format on
    return true;
}

bool Engine::Frame() {

    //update
    black_board->job_context->dt = ImGui::GetIO().DeltaTime;
    black_board->job_context->stage_type = EnumStageType::eUpdate;
    update_tree->Run();

    //render
    black_board->job_context->stage_type = EnumStageType::eRender;
    render_tree->Run();

    return true;
}

bool Engine::Stop() {
    Platform::Stop();

    // if (black_board) {

    //    if (black_board->job_context) {
    //        for (auto &model : black_board->job_context->objects) {
    //            model.second.reset();
    //        }
    //        black_board->job_context->camera.reset();
    //    }

    //    if (black_board->gui) {
    //        black_board->gui->Shutdown();
    //        black_board->gui.reset();
    //    }

    //    if (black_board->input) {
    //        black_board->input.reset();
    //    }
    //}

    return true;
}

} // namespace core