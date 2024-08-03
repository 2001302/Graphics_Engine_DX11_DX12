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
            ->Excute(ImGuiNode())
            ->Excute(InitializeLight())
            ->Excute(CameraNode())
            ->Excute(InitializeSkybox())
            ->Excute(InitializeMirrorGround())
            ->Excute(CreateGlobalConstantBuffer())
            ->Excute(PostProcessing())
            ->Excute(InitializeBasicModels())
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
            ->Excute(UpdateLights())
            ->Excute(UpdateGlobalConstantBuffers())
            ->Excute(UpdateMirror())
            ->Excute(UpdateBasicObjects())
            ->Excute(UpdateLightSpheres())
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
            ->Excute(SetSamplerStates())
            ->Excute(DrawOnlyDepth())
            ->Excute(SetShadowViewport())
            ->Excute(DrawShadowMap())
            ->Excute(SetMainRenderTarget())
            ->Excute(DrawObjects())
            ->Excute(DrawLightSpheres())
            ->Excute(CameraNode())
            ->Excute(DrawSkybox())
            ->Excute(DrawMirrorSurface())
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

    //if (render_block) {
    //    for (auto &model : render_block->models) {
    //        model.second.reset();
    //    }
    //    render_block->camera.reset();
    //}

    //if (gui) {
    //    gui->Shutdown();
    //    gui.reset();
    //}

    //if (input) {
    //    input.reset();
    //}

    return true;
}

} // namespace engine