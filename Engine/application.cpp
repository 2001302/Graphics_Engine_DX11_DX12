#include "application.h"
#include "behavior_tree_builder.h"
#include "renderer_initialize_node.h"
#include "renderer_update_node.h"
#include "renderer_draw_node.h"
#include "gui_node.h"
#include "input_node.h"

namespace engine {

Application::Application() {
    manager_ = std::make_shared<PipelineManager>();
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
            delete model.second;
            model.second = 0;
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

bool CheckIfMouseInViewport() {

    if (GraphicsManager::Instance().swap_chain) {

        auto cursor = ImGui::GetMousePos();

        auto left = GraphicsManager::Instance().viewport.TopLeftX +
                    ImGui::GetWindowSize().x;
        auto top = GraphicsManager::Instance().viewport.TopLeftY;
        auto right = GraphicsManager::Instance().viewport.Width + left;
        auto bottom = GraphicsManager::Instance().viewport.Height + top;

        if (left < cursor.x && cursor.x < right && top < cursor.y &&
            cursor.y < bottom) {
            return true;
        }

        return false;
    }
}

LRESULT CALLBACK Application::MessageHandler(HWND main_window, UINT umsg,
                                             WPARAM wparam, LPARAM lparam) {
    extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
        HWND main_window, UINT msg, WPARAM wParam, LPARAM lParam);

    ImGui_ImplWin32_WndProcHandler(main_window, umsg, wparam, lparam);

    switch (umsg) {
    case WM_MOUSEMOVE: {
        if (wparam & MK_RBUTTON) {
            // if (CheckIfMouseInViewport())
            {
                return message_receiver_->OnMouseRightDragRequest(
                    manager_.get(), input_);
            }
        }
        if (wparam & MK_MBUTTON) {
            // if (CheckIfMouseInViewport())
            {
                return message_receiver_->OnMouseWheelDragRequest(
                    manager_.get(), input_, LOWORD(lparam), HIWORD(lparam));
            }
        }
        break;
    }
    case WM_MOUSEWHEEL: {
        // if (CheckIfMouseInViewport())
        {
            return message_receiver_->OnMouseWheelRequest(manager_.get(),
                                                          input_);
        }
        break;
    }
    case WM_RBUTTONDOWN: {
        // return OnRightClickRequest();
        break;
    }
    case WM_LBUTTONUP: {
        break;
    }
    case WM_MODEL_LOAD: {
        return message_receiver_->OnModelLoadRequest(manager_.get(),
                                                     main_window);
        break;
    }
    case WM_SPHERE_LOAD: {
        return message_receiver_->OnSphereLoadRequest(manager_.get());
        break;
    }
    case WM_BOX_LOAD: {
        return message_receiver_->OnBoxLoadRequest(manager_.get());
        break;
    }
    case WM_CYLINDER_LOAD: {
        return message_receiver_->OnCylinderLoadRequest(manager_.get());
        break;
    }
    case WM_SIZE: {
        if (GraphicsManager::Instance().swap_chain) {

            imgui_->Shutdown();

            common::Env::Instance().screen_width = int(LOWORD(lparam));
            common::Env::Instance().screen_height = int(HIWORD(lparam));

            GraphicsManager::Instance().back_buffer_RTV.Reset();
            GraphicsManager::Instance().swap_chain->ResizeBuffers(
                0, (UINT)LOWORD(lparam), (UINT)HIWORD(lparam),
                DXGI_FORMAT_UNKNOWN, 0);

            GraphicsManager::Instance().CreateBuffer();
            GraphicsManager::Instance().SetMainViewport();

            imgui_->Initialize();

            manager_->m_postProcess.Initialize(
                GraphicsManager::Instance().device,
                GraphicsManager::Instance().device_context,
                {GraphicsManager::Instance().postEffectsSRV},
                {GraphicsManager::Instance().back_buffer_RTV},
                common::Env::Instance().screen_width,
                common::Env::Instance().screen_height, 4);
        }

        break;
    }
    default: {
        return DefWindowProc(main_window, umsg, wparam, lparam);
    }
    }
}
} // namespace engine