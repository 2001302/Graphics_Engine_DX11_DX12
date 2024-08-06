#include "application.h"
#include "behavior_tree_builder.h"

namespace engine {

Application::Application() {
    black_board = std::make_shared<BlackBoard>();
    message_receiver = std::make_unique<MessageReceiver>();
};

bool Application::OnStart() {

    Platform::OnStart();

    GraphicsManager::Instance().Initialize();
    black_board->render_block->stage_type = EnumStageType::eInitialize;

    // clang-format off
    auto tree = std::make_shared<common::BehaviorTreeBuilder>();
    tree->Build(black_board.get())
        ->Sequence()
            ->Excute(ShadowEffectNode())
            ->Excute(ImGuiNode())
            ->Excute(LightNode())
            ->Excute(CameraNode())
            ->Excute(SkyboxNode())
            ->Excute(MirrorEffectNode())
            ->Excute(SharedResourceNode())
            ->Excute(PostProcessing())
            ->Excute(GameObjectsNode())
            ->Excute(PlayerNode())
        ->Close()
    ->Run();
    // clang-format on

    OnFrame();

    return true;
}

bool Application::OnFrame() {

    OnUpdate(ImGui::GetIO().DeltaTime);
    OnRender();

    return true;
}

bool Application::OnUpdate(float dt) {

    black_board->render_block->dt = dt;
    black_board->render_block->stage_type = EnumStageType::eUpdate;

    // clang-format off
    auto tree = std::make_shared<common::BehaviorTreeBuilder>();
    tree->Build(black_board.get())
        ->Sequence()
            ->Excute(CameraNode())
            ->Excute(LightNode())
            ->Excute(SharedResourceNode())
            ->Excute(ShadowEffectNode())
            ->Excute(MirrorEffectNode())
            ->Excute(GameObjectsNode())
            ->Excute(PlayerNode())
        ->Close()
    ->Run();
    // clang-format on

    return true;
}

bool Application::OnRender() {

    //input->Frame();
    black_board->render_block->stage_type = EnumStageType::eRender;

    // clang-format off
    auto tree = std::make_shared<common::BehaviorTreeBuilder>();
    tree->Build(black_board.get())
        ->Sequence()
            ->Excute(SharedResourceNode())
            ->Excute(ShadowEffectNode())
            ->Excute(GameObjectsNode())
            ->Excute(PlayerNode())
            ->Excute(CameraNode())
            ->Excute(SkyboxNode())
            ->Excute(MirrorEffectNode())
            ->Excute(ResolveBuffer())
            ->Excute(PostProcessing())
            ->Excute(ImGuiNode())
            ->Excute(Present())
        ->Close()
    ->Run();
    // clang-format on

    return true;
}

bool Application::OnStop() {
    Platform::OnStop();

    if (black_board) {

        if (black_board->render_block) {
            for (auto &model : black_board->render_block->models) {
                model.second.reset();
            }
            black_board->render_block->camera.reset();
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