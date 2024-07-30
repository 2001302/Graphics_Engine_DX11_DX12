#include "application.h"
#include "behavior_tree_builder.h"
#include "gui_node.h"
#include "input_node.h"
#include "renderer_draw_node.h"
#include "renderer_initialize_node.h"
#include "renderer_update_node.h"

namespace engine {

Application::Application() {
    manager_ = std::make_shared<RenderingBlock>();
    message_receiver_ = std::make_unique<MessageReceiver>();
    input_ = std::make_unique<Input>();
    imgui_ = std::make_shared<common::SettingUi>();
};

bool Application::OnStart() {

    Platform::OnStart();

    std::map<EnumDataBlockType, common::IDataBlock *> dataBlock = {
        {EnumDataBlockType::eManager, manager_.get()},
        {EnumDataBlockType::eGui, imgui_.get()},
        {EnumDataBlockType::eInput, input_.get()},
    };

    // clang-format off
    auto tree = std::make_shared<BehaviorTreeBuilder>();
    tree->Build(dataBlock)
        ->Sequence()
            ->Excute(std::make_shared<InitializeInputNode>(hinstance_))
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

    std::map<EnumDataBlockType, common::IDataBlock *> dataBlock = {
        {EnumDataBlockType::eManager, manager_.get()},
        {EnumDataBlockType::eGui, imgui_.get()},
        {EnumDataBlockType::eInput, input_.get()},
    };

    // clang-format off
    auto tree = std::make_shared<BehaviorTreeBuilder>();
    tree->Build(dataBlock)
        ->Sequence()
            ->Excute(ReadInput())
            ->Excute(UpdateCamera())
            ->Excute(std::make_shared<UpdateLightsNode>(dt))
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

    input_->Frame();

    std::map<EnumDataBlockType, common::IDataBlock *> dataBlock = {
        {EnumDataBlockType::eManager, manager_.get()},
        {EnumDataBlockType::eGui, imgui_.get()},
        {EnumDataBlockType::eInput, input_.get()},
    };

    // clang-format off
    auto tree = std::make_shared<BehaviorTreeBuilder>();
    tree->Build(dataBlock)
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
    ->Run();
    // clang-format on

    return true;
}

bool Application::OnStop() {
    Platform::OnStop();

    if (manager_) {
        for (auto &model : manager_->models) {
            model.second.reset();
        }
        manager_->camera.reset();
    }

    if (imgui_) {
        imgui_->Shutdown();
        imgui_.reset();
    }

    if (input_) {
        input_->Shutdown();
        input_.reset();
    }

    return true;
}

} // namespace engine