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
            ->Excute(std::make_shared<InitializeInputNode>(hinstance_))
            ->Excute(std::make_shared<InitializeImguiNode>())
            ->Excute(std::make_shared<InitializeLightNode>())
            ->Excute(std::make_shared<InitializeCameraNode>())
            ->Excute(std::make_shared<InitializeSkyboxNode>())
            ->Excute(std::make_shared<InitializeMirrorGroundNode>())
            ->Excute(std::make_shared<CreateGlobalConstantBufferNode>())
            ->Excute(std::make_shared<InitializePostEffectNode>())
            ->Excute(InitializePostProcessing())
            ->Excute(std::make_shared<InitializeBasicModelsNode>())
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
            ->Excute(std::make_shared<ReadInputNode>())
            ->Excute(std::make_shared<UpdateCameraNode>())
            ->Excute(std::make_shared<UpdateLightsNode>(dt))
            ->Excute(std::make_shared<UpdateGlobalConstantBuffersNode>())
            ->Excute(std::make_shared<UpdateMirrorNode>())
            //->Excute(std::make_shared<ApplyMouseMovement>())
            ->Excute(std::make_shared<UpdateBasicObjectsNode>())
            ->Excute(std::make_shared<UpdateLightSpheresNode>())
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
        ->Excute(std::make_shared<SetSamplerStatesNode>())
        ->Excute(std::make_shared<DrawOnlyDepthNode>())
        ->Excute(std::make_shared<SetShadowViewportNode>())
        ->Excute(std::make_shared<DrawShadowMapNode>())
        ->Excute(std::make_shared<SetMainRenderTargetNode>())
        ->Excute(std::make_shared<DrawObjectsNode>())
        ->Excute(std::make_shared<DrawLightSpheresNode>())
        ->Excute(std::make_shared<DrawRelatedWithCameraNode>())
        ->Excute(std::make_shared<DrawSkyboxNode>())
        ->Excute(std::make_shared<DrawMirrorSurfaceNode>())
        ->Excute(std::make_shared<ResolveBufferNode>())
        ->Excute(DrawPostProcessing())
        ->Excute(std::make_shared<DrawSettingUiNode>())
        ->Excute(std::make_shared<PresentNode>())
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