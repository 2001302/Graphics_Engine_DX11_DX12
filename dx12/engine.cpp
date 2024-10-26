#include "engine.h"

namespace graphics {
Engine::Engine() {
    black_board = std::make_shared<BlackBoard>();
    message_receiver = std::make_unique<MessageReceiver>();
    time_stamp = std::make_unique<common::TimeStamp>();
    start_tree = std::make_shared<common::BehaviorTreeBuilder>();
    update_tree = std::make_shared<common::BehaviorTreeBuilder>();
    render_tree = std::make_shared<common::BehaviorTreeBuilder>();
};

bool Engine::Start() {
    // clang-format off
    Platform::Start();

    GpuCore::Instance().Initialize();
    GpuBuffer::Instance().Initialize();

    LoadAsset(black_board.get());

    black_board->conditions->stage_type = EnumStageType::eInitialize;

    //initialize
    start_tree->Build(black_board.get())
    ->Sequence()
        ->Excute(node::global_constant_node)
        ->Excute(node::camera_node)
        ->Excute(node::light_node)
        ->Excute(node::mesh_object_node)
        ->Excute(node::skybox_node)
        ->Excute(node::shadow_effect_node)
        ->Excute(node::mirror_object_node)
        ->Excute(node::tone_mapping)
        ->Excute(node::gui_node)
    ->Close()
    ->Run();
    
    //update
    update_tree->Build(black_board.get())
    ->Sequence()
        ->Excute(node::camera_node)
        ->Excute(node::light_node)
        ->Excute(node::global_constant_node)
        ->Excute(node::mesh_object_node)
        ->Excute(node::shadow_effect_node)
        ->Excute(node::mirror_object_node)
    ->Close();
     
    //render
    render_tree->Build(black_board.get())
    ->Sequence()
        ->Excute(node::clear_buffer)
        ->Excute(node::shadow_effect_node)
        ->Excute(node::mesh_object_node)
        ->Excute(node::skybox_node)
        ->Excute(node::mirror_object_node)
        ->Excute(node::tone_mapping)
        ->Excute(node::gui_node)
        ->Excute(node::present)
    ->Close();

    Frame();

    // clang-format on
    return true;
}

bool Engine::Frame() {

    // update
    {
        common::ScopeStopWatch stop_watch("Update tree");
        black_board->conditions->delta_time = time_stamp->DeltaTime();
        black_board->conditions->stage_type = EnumStageType::eUpdate;
        update_tree->Run();
    }

    // render
    {
        common::ScopeStopWatch stop_watch("Render tree");
        black_board->conditions->stage_type = EnumStageType::eRender;
        render_tree->Run();
    }

    return true;
}

bool Engine::Stop() {
    Platform::Stop();

    if (black_board) {
        if (black_board->targets) {
            for (auto &model : black_board->targets->objects) {
                model.second.reset();
            }
            black_board->targets->camera.reset();
        }
        if (black_board->gui) {
            GuiNodeInvoker::Shutdown();
            black_board->gui.reset();
        }
        if (black_board->input) {
            black_board->input.reset();
        }
    }

    GpuCore::Instance().Shutdown();

    return true;
}
} // namespace graphics
