#include "engine.h"

#include "camera_node.h"
#include "game_objects_node.h"
#include "gpu_node.h"
#include "gui_node.h"
#include "light_node.h"
#include "skybox_node.h"
#include "tone_mapping_node.h"

namespace graphics {

auto gpu_initialize = std::make_shared<GpuInitializeNode>();
auto clear_buffer = std::make_shared<ClearBufferNode>();
auto camera_node = std::make_shared<CameraNodeInvoker>();
auto global_resource_node = std::make_shared<GlobalResourceNode>();
auto global_constant_node = std::make_shared<GlobalConstantNode>();
auto game_object_node = std::make_shared<GameObjectNodeInvoker>();
auto gui_node = std::make_shared<GuiNodeInvoker>();
auto present = std::make_shared<PresentNode>();
auto light_node = std::make_shared<LightNodeInvoker>();
auto resolve_buffer = std::make_shared<ResolveBuffer>();
auto tone_mapping = std::make_shared<ToneMappingNodeInvoker>();
auto begin_init = std::make_shared<BeginInitNode>();
auto end_init = std::make_shared<EndInitNode>();
auto skybox_node = std::make_shared<SkyBoxNodeInvoker>();

Engine::Engine() {
    black_board = std::make_shared<BlackBoard>();
    message_receiver = std::make_unique<MessageReceiver>();
    start_tree = std::make_shared<common::BehaviorTreeBuilder>();
    update_tree = std::make_shared<common::BehaviorTreeBuilder>();
    render_tree = std::make_shared<common::BehaviorTreeBuilder>();
};

bool Engine::Start() {
    // clang-format off
    Platform::Start();
    
    black_board->conditions->stage_type = EnumStageType::eInitialize;

    //initialize
    start_tree->Build(black_board.get())
    ->Sequence()
        ->Excute(gpu_initialize)
        ->Excute(global_resource_node)
        ->Excute(global_constant_node)
        ->Excute(camera_node)
        ->Excute(light_node)
        ->Excute(game_object_node)
        ->Excute(skybox_node)
        ->Excute(tone_mapping)
        ->Excute(gui_node)
    ->Close()
    ->Run();
    
    //update
    update_tree->Build(black_board.get())
    ->Sequence()
        ->Excute(camera_node)
        ->Excute(light_node)
        ->Excute(global_resource_node)
        ->Excute(global_constant_node)
        ->Excute(game_object_node)
    ->Close();
     
    //render
    render_tree->Build(black_board.get())
    ->Sequence()
        ->Excute(clear_buffer)
        ->Excute(skybox_node)
        ->Excute(game_object_node)
        ->Excute(tone_mapping)
        ->Excute(gui_node)
        ->Excute(present)
    ->Close();

    Frame();

    // clang-format on
    return true;
}

bool Engine::Frame() {

    // update
    black_board->conditions->delta_time = ImGui::GetIO().DeltaTime;
    black_board->conditions->stage_type = EnumStageType::eUpdate;
    update_tree->Run();

    // render
    black_board->conditions->stage_type = EnumStageType::eRender;
    render_tree->Run();

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
