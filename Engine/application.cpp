#include "application.h"
#include "behavior_tree_builder.h"
#include "gui_node.h"
#include "input_node.h"
#include "renderer_draw_node.h"
#include "renderer_initialize_node.h"
#include "renderer_update_node.h"

namespace engine {

Application::Application() {
    render_block = std::make_shared<RenderingBlock>();
    message_receiver = std::make_unique<MessageReceiver>();
    input = std::make_unique<Input>();
    gui = std::make_shared<common::SettingUi>();
};

bool Application::OnStart() {

    Platform::OnStart();

    input->Initialize(hinstance);

    std::map<common::EnumDataBlockType, common::IDataBlock *> dataBlock = {
        {common::EnumDataBlockType::eRenderBlock, render_block.get()},
        {common::EnumDataBlockType::eGui, gui.get()},
        {common::EnumDataBlockType::eInput, input.get()},
    };

    // clang-format off
    auto tree = std::make_shared<common::BehaviorTreeBuilder>();
    tree->Build(dataBlock)
        ->Sequence()
            ->Excute(InitializeImgui())
            ->Excute(InitializeLight())
            ->Excute(InitializeCamera())
            ->Excute(InitializeSkybox())
            ->Excute(InitializeMirrorGround())
            ->Excute(CreateGlobalConstantBuffer())
            ->Excute(InitializePostEffect())
            ->Excute(InitializePostProcessing())
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

    render_block->dt = dt;

    std::map<common::EnumDataBlockType, common::IDataBlock *> dataBlock = {
        {common::EnumDataBlockType::eRenderBlock, render_block.get()},
        {common::EnumDataBlockType::eGui, gui.get()},
        {common::EnumDataBlockType::eInput, input.get()},
    };

    // clang-format off
    auto tree = std::make_shared<common::BehaviorTreeBuilder>();
    tree->Build(dataBlock)
        ->Sequence()
            ->Excute(ReadInput())
            ->Excute(UpdateCamera())
            ->Excute(UpdateLights())
            ->Excute(UpdateGlobalConstantBuffers())
            ->Excute(UpdateMirror())
            //->Excute(ApplyMouseMove())
            ->Excute(UpdateBasicObjects())
            ->Excute(UpdateLightSpheres())
        ->Close()
    ->Run();
    // clang-format on

    return true;
}

bool Application::OnRender() {

    input->Frame();

    std::map<common::EnumDataBlockType, common::IDataBlock *> dataBlock = {
        {common::EnumDataBlockType::eRenderBlock, render_block.get()},
        {common::EnumDataBlockType::eGui, gui.get()},
        {common::EnumDataBlockType::eInput, input.get()},
    };

    // clang-format off
    auto tree = std::make_shared<common::BehaviorTreeBuilder>();
    tree->Build(dataBlock)
        ->Sequence()
            ->Excute(SetSamplerStates())
            ->Excute(DrawOnlyDepth())
            ->Excute(SetShadowViewport())
            ->Excute(DrawShadowMap())
            ->Excute(SetMainRenderTarget())
            ->Excute(DrawObjects())
            ->Excute(DrawLightSpheres())
            ->Excute(DrawRelatedWithCamera())
            ->Excute(DrawSkybox())
            ->Excute(DrawMirrorSurface())
            ->Excute(ResolveBuffer())
            ->Excute(DrawPostProcessing())
            ->Excute(DrawSettingUi())
            ->Excute(Present())
        ->Close()
    ->Run();
    // clang-format on

    return true;
}

bool Application::OnStop() {
    Platform::OnStop();

    if (render_block) {
        for (auto &model : render_block->models) {
            model.second.reset();
        }
        render_block->camera.reset();
    }

    if (gui) {
        gui->Shutdown();
        gui.reset();
    }

    if (input) {
        input->Shutdown();
        input.reset();
    }

    return true;
}

} // namespace engine