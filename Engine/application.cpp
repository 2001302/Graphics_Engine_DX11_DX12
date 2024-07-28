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
    auto tree = new BehaviorTreeBuilder();
    tree->Build(dataBlock)
        ->Sequence()
            ->Excute(std::make_shared<InitializeInput>(hinstance_))
            ->Excute(std::make_shared<InitializeImgui>())
            ->Excute(std::make_shared<InitializeLight>())
            ->Excute(std::make_shared<InitializeCamera>())
            ->Excute(std::make_shared<InitializeSkybox>())
            ->Excute(std::make_shared<InitializeMirrorGround>())
            ->Excute(std::make_shared<CreateGlobalConstantBuffer>())
            ->Excute(std::make_shared<InitializePostEffect>())
            ->Excute(std::make_shared<InitializePostProcessing>())
            ->Excute(std::make_shared<InitializeBasicModels>())
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
    auto tree = new BehaviorTreeBuilder();
    tree->Build(dataBlock)
        ->Sequence()
            ->Excute(std::make_shared<ReadInput>())
            ->Excute(std::make_shared<UpdateCamera>())
            ->Excute(std::make_shared<UpdateLights>(dt))
            ->Excute(std::make_shared<UpdateGlobalConstantBuffers>())
            ->Excute(std::make_shared<UpdateMirror>())
            //->Excute(std::make_shared<ApplyMouseMovement>())
            ->Excute(std::make_shared<UpdateBasicObjects>())
            ->Excute(std::make_shared<UpdateLightSpheres>())
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
    auto tree = std::make_unique<BehaviorTreeBuilder>();
    tree->Build(dataBlock)
        ->Excute(std::make_shared<SetSamplerStates>())
        ->Excute(std::make_shared<DrawOnlyDepth>())
        ->Excute(std::make_shared<SetShadowViewport>())
        ->Excute(std::make_shared<DrawShadowMap>())
        ->Excute(std::make_shared<SetMainRenderTarget>())
        ->Excute(std::make_shared<DrawObjects>())
        ->Excute(std::make_shared<DrawLightSpheres>())
        ->Excute(std::make_shared<DrawRelatedWithCamera>())
        ->Excute(std::make_shared<DrawSkybox>())
        ->Excute(std::make_shared<DrawMirrorSurface>())
        ->Excute(std::make_shared<ResolveBuffer>())
        ->Excute(std::make_shared<DrawPostProcessing>())
        ->Excute(std::make_shared<DrawSettingUi>())
        ->Excute(std::make_shared<Present>())
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