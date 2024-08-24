#include "engine.h"
#include "../foundation/behavior_tree_builder.h"

namespace core {

auto gpu_initialize = std::make_shared<GpuInitializeNode>();
auto begin_render = std::make_shared<BeginRenderNode>();
auto end_render = std::make_shared<EndRenderNode>();
auto camera_node = std::make_shared<CameraNodeInvoker>();
auto shared_resource_node = std::make_shared<SharedResourceNodeInvoker>();
auto game_object_node = std::make_shared<GameObjectNodeInvoker>();
auto gui_node = std::make_shared<GuiNodeInvoker>();
auto present = std::make_shared<PresentNode>();
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
    
    black_board->render_condition->stage_type = EnumStageType::eInitialize;
    
    //initialize
    start_tree->Build(black_board.get())
    ->Sequence()
        ->Excute(gpu_initialize)
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
        ->Excute(begin_render)
        ->Excute(game_object_node)
        ->Excute(gui_node)
        ->Excute(end_render)
        ->Excute(present)
    ->Close();

    Frame();

    // clang-format on
    return true;
}

bool Engine::Frame() {

    // update
    black_board->render_condition->dt = ImGui::GetIO().DeltaTime;
    black_board->render_condition->stage_type = EnumStageType::eUpdate;
    update_tree->Run();

    // render
    black_board->render_condition->stage_type = EnumStageType::eRender;
    render_tree->Run();

    return true;
}

bool Engine::Stop() {
    Platform::Stop();

    if (black_board) {
        if (black_board->render_targets) {
            for (auto &model : black_board->render_targets->objects) {
                model.second.reset();
            }
            black_board->render_targets->camera.reset();
        }
        if (black_board->render_condition->gui) {
            black_board->render_condition->gui->Shutdown();
            black_board->render_condition->gui.reset();
        }
        if (black_board->render_condition->input) {
            black_board->render_condition->input.reset();
        }
    }

    return true;
}

} // namespace core
