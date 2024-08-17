#include "engine.h"
#include "../foundation/behavior_tree_builder.h"
#include "gpu_node.h"
#include "gui_node.h"

namespace core {

Engine::Engine() {
    black_board = std::make_shared<BlackBoard>();
    // message_receiver = std::make_unique<MessageReceiver>();
    gui = std::make_shared<foundation::SettingUi>();
};

bool Engine::Start() {
    // clang-format off

    Platform::Start();

    dx12::GpuCore::Instance().Initialize();
    gui->Start();
    black_board->job_context->stage_type = EnumStageType::eInitialize;

     auto tree = std::make_shared<foundation::BehaviorTreeBuilder>();
     tree->Build(black_board.get())
         ->Sequence()
             ->Excute(std::make_shared<GuiNodeInvoker>())
         ->Close()
    ->Run();

    Frame();

    // clang-format on
    return true;
}

bool Engine::Frame() {
    // clang-format off

     black_board->job_context->dt = ImGui::GetIO().DeltaTime;
     black_board->job_context->stage_type = EnumStageType::eUpdate;

     auto update_tree = std::make_shared<foundation::BehaviorTreeBuilder>();
     update_tree->Build(black_board.get())
         ->Sequence()
             //->Excute(camera_node)
         ->Close()
    ->Run();

    black_board->job_context->stage_type = EnumStageType::eRender;

    auto render_tree = std::make_shared<foundation::BehaviorTreeBuilder>();
    render_tree->Build(black_board.get())
         ->Sequence()
             ->Excute(std::make_shared<StartRenderingNode>())
             ->Excute(std::make_shared<GuiNodeInvoker>())
             ->Excute(std::make_shared<PresentNode>())
         ->Close()
    ->Run();

    // clang-format on
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