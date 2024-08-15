#include "engine.h"
#include "behavior_tree_builder.h"

namespace core {

Engine::Engine() {
    black_board = std::make_shared<BlackBoard>();
    message_receiver = std::make_unique<MessageReceiver>();
};

bool Engine::Start() {

    Platform::Start();

    graphics::GpuCore::Instance().Initialize();
    black_board->job_context->stage_type = EnumStageType::eInitialize;

    // clang-format off
    auto tree = std::make_shared<common::BehaviorTreeBuilder>();
    tree->Build(black_board.get())
        ->Sequence()
            ->Excute(shadow_effect_node)
            ->Excute(light_node)
            ->Excute(camera_node)
            ->Excute(skybox_node)
            ->Excute(mirror_effect_node)
            ->Excute(shared_resource_node)
            ->Excute(post_processing)
            ->Excute(game_objects_node)
            ->Excute(player_node)
            ->Excute(imgui_node)
        ->Close()
    ->Run();
    // clang-format on

    OnFrame();

    return true;
}

bool Engine::OnFrame() {

    OnUpdate(ImGui::GetIO().DeltaTime);
    OnRender();

    return true;
}

bool Engine::OnUpdate(float dt) {

    black_board->job_context->dt = dt;
    black_board->job_context->stage_type = EnumStageType::eUpdate;

    // clang-format off
    auto tree = std::make_shared<common::BehaviorTreeBuilder>();
    tree->Build(black_board.get())
        ->Sequence()
            ->Excute(camera_node)
            ->Excute(light_node)
            ->Excute(shared_resource_node)
            ->Excute(shadow_effect_node)
            ->Excute(mirror_effect_node)
            ->Excute(game_objects_node)
            ->Excute(player_node)
        ->Close()
    ->Run();
    // clang-format on

    return true;
}

bool Engine::OnRender() {

    black_board->job_context->stage_type = EnumStageType::eRender;

    // clang-format off
    auto tree = std::make_shared<common::BehaviorTreeBuilder>();
    tree->Build(black_board.get())
        ->Sequence()
            ->Excute(shared_resource_node)
            ->Excute(shadow_effect_node)
            ->Excute(light_node)
            ->Excute(game_objects_node)
            ->Excute(player_node)
            ->Excute(camera_node)
            ->Excute(skybox_node)
            ->Excute(mirror_effect_node)
            ->Excute(resolve_buffer)
            ->Excute(post_processing)
            ->Excute(imgui_node)
            ->Excute(present)
        ->Close()
    ->Run();
    // clang-format on

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

} // namespace engine