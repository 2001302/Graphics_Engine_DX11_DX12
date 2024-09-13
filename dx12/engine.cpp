#include "engine.h"

#include "camera_node.h"
#include "game_objects_node.h"
#include "gpu_node.h"
#include "gui_node.h"
#include "light_node.h"
#include "shared_resource_node.h"
#include "skybox_node.h"
#include "tone_mapping_node.h"

namespace graphics {

auto gpu_initialize = std::make_shared<GpuInitializeNode>();
auto begin_render = std::make_shared<BeginRenderNode>();
auto end_render = std::make_shared<EndRenderNode>();
auto camera_node = std::make_shared<CameraNodeInvoker>();
auto shared_resource_node = std::make_shared<SharedResourceNodeInvoker>();
auto game_object_node = std::make_shared<GameObjectNodeInvoker>();
auto gui_node = std::make_shared<GuiNodeInvoker>();
auto present = std::make_shared<PresentNode>();
auto fence = std::make_shared<GpuFenceNode>();
auto light_node = std::make_shared<LightNodeInvoker>();
auto resolve_buffer = std::make_shared<ResolveBuffer>();
auto tone_mapping = std::make_shared<ToneMappingNodeInvoker>();
auto begin_init = std::make_shared<BeginInitNode>();
auto end_init = std::make_shared<EndInitNode>();
auto skybox_node = std::make_shared<SkyBoxNodeInvoker>();


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
    
    black_board->conditions->stage_type = EnumStageType::eInitialize;

    //initialize
    start_tree->Build(black_board.get())
    ->Sequence()
        ->Excute(gpu_initialize)
        ->Excute(begin_init)
        //->Excute(camera_node)
        //->Excute(light_node)
        //->Excute(shared_resource_node)
        //->Excute(game_object_node)
        //->Excute(tone_mapping)
        //->Excute(skybox_node)
        ->Excute(gui_node)
        ->Excute(end_init)
        ->Excute(fence)
    ->Close()
    ->Run();
    
    //update
    update_tree->Build(black_board.get())
    ->Sequence()
        //->Excute(camera_node)
        //->Excute(light_node)
        //->Excute(shared_resource_node)
        //->Excute(game_object_node)
    ->Close();
     
    //render
    render_tree->Build(black_board.get())
    ->Sequence()
        ->Excute(begin_render)
        //->Excute(skybox_node)
        //->Excute(game_object_node)
        ->Excute(resolve_buffer)
        //->Excute(tone_mapping)
        ->Excute(gui_node)
        ->Excute(end_render)
        ->Excute(present)
        ->Excute(fence)
    ->Close();

    Frame();

    // clang-format on
    return true;
}

bool Engine::Frame() {

    // update
    black_board->conditions->dt = ImGui::GetIO().DeltaTime;
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
