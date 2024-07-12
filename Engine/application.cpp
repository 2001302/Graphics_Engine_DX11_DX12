#include "application.h"
#include "behavior_tree_builder.h"

namespace platform {

Application::Application() : imgui_(0), manager_(0) {
    input_ = std::make_unique<Engine::Input>();
    manager_ = std::make_shared<Engine::PipelineManager>();
    message_receiver_ = std::make_unique<Engine::MessageReceiver>();
    imgui_ = std::make_shared<Engine::SettingUi>(manager_);
};

bool Application::OnStart() {

    Platform::OnStart();

    std::map<Engine::EnumDataBlockType, Engine::IDataBlock *> dataBlock = {
        {Engine::EnumDataBlockType::eManager, manager_.get()},
        {Engine::EnumDataBlockType::eGui, imgui_.get()},
    };

    Engine::Direct3D::GetInstance().Initialize();

    input_->Initialize(hinstance_, Engine::Env::Get().screen_width,
                       Engine::Env::Get().screen_height);
    imgui_->Initialize();

    // clang-format off
    auto tree = new Engine::BehaviorTreeBuilder();
    tree->Build(dataBlock)
        ->Sequence()
            ->Excute(std::make_shared<Engine::InitializeBoardMap>())
            ->Excute(std::make_shared<Engine::InitializeCamera>())
            ->Excute(std::make_shared<Engine::InitializeGroundShader>())
            ->Excute(std::make_shared<Engine::InitializeCubeMapShader>())
            ->Excute(std::make_shared<Engine::InitializePhongShader>())
            ->Excute(std::make_shared<Engine::InitializeImageBasedShader>())
            ->Excute(std::make_shared<Engine::InitializePhysicallyBasedShader>())
        ->Close()
    ->Run();
    // clang-format on

    OnFrame();

    return true;
}

bool Application::OnFrame() {

    std::map<Engine::EnumDataBlockType, Engine::IDataBlock *> dataBlock = {
        {Engine::EnumDataBlockType::eManager, manager_.get()},
        {Engine::EnumDataBlockType::eGui, imgui_.get()},
    };

    // Clear the buffers to begin the scene.
    Engine::Direct3D::GetInstance().BeginScene(
        0.0f, 0.0f, 0.0f, 1.0f,
                                       imgui_->GetGlobalTab().draw_as_wire_);

    // clang-format off
    std::vector<int> model_ids;
    for(auto model : manager_->models)
        model_ids.push_back(model.first);
    
    auto tree = std::make_unique<Engine::BehaviorTreeBuilder>();
    tree->Build(dataBlock)
    ->Excute(std::make_shared<Engine::UpdateCamera>())
    ->Parallel(model_ids)
        ->Conditional(std::make_shared<Engine::CheckImageBasedShader>())
            ->Sequence()
                ->Excute(std::make_shared<Engine::UpdateGameObjectsUsingImageBasedShader>())
                ->Excute(std::make_shared<Engine::RenderGameObjectsUsingImageBasedShader>())
            ->Close()
        ->End()
        ->Conditional(std::make_shared<Engine::CheckPhongShader>())
            ->Sequence()
                ->Excute(std::make_shared<Engine::UpdateGameObjectsUsingPhongShader>())
                ->Excute(std::make_shared<Engine::RenderGameObjectsUsingPhongShader>())
            ->Close()
        ->End()
        ->Conditional(std::make_shared<Engine::CheckPhysicallyBasedShader>())
            ->Sequence()
                ->Excute(std::make_shared<Engine::UpdateGameObjectsUsingPhysicallyBasedShader>())
                ->Excute(std::make_shared<Engine::RenderGameObjectsUsingPhysicallyBasedShader>())
            ->Close()
        ->End()
    ->Close()
    ->Conditional(std::make_shared<Engine::CheckGroundShader>())
        ->Sequence()
            ->Excute(std::make_shared<Engine::UpdateGroundShader>())
            ->Excute(std::make_shared<Engine::RenderGroundShader>())
        ->Close()
    ->End()
    ->Conditional(std::make_shared<Engine::CheckCubeMapShader>())
        ->Sequence()
            ->Excute(std::make_shared<Engine::UpdateCubeMap>())
            ->Excute(std::make_shared<Engine::RenderCubeMap>())
        ->Close()
    ->End()
    ->Excute(std::make_shared<Engine::RenderBoardMap>())
    ->Run();
    // clang-format on

    input_->Frame();
    imgui_->Frame();

    // Present the rendered scene to the screen.
    Engine::Direct3D::GetInstance().EndScene();

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

bool CheckIfMouseInViewport() {
    auto cursor = ImGui::GetMousePos();
    auto view_port = Engine::Direct3D::GetInstance().viewport();
    if (view_port.TopLeftX < cursor.x && view_port.TopLeftY < cursor.y &&
        cursor.x < view_port.TopLeftX + view_port.Width &&
        cursor.y < view_port.TopLeftY + view_port.Height) {
        return true;
    }
    return false;
}

LRESULT CALLBACK Application::MessageHandler(HWND main_window, UINT umsg,
                                             WPARAM wparam, LPARAM lparam) {
    extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
        HWND main_window, UINT msg, WPARAM wParam, LPARAM lParam);

    ImGui_ImplWin32_WndProcHandler(main_window, umsg, wparam, lparam);

    switch (umsg) {
    case WM_MOUSEMOVE: {
        if (CheckIfMouseInViewport()) {
            if (wparam & MK_RBUTTON) {
                return message_receiver_->OnRightDragRequest(manager_.get(),
                                                             input_);
            }
        }
        break;
    }
    case WM_MOUSEWHEEL: {
        if (CheckIfMouseInViewport()) {
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
    /* case WM_SIZE: {
        if (Direct3D::GetInstance().swap_chain()) {

            imgui_->Shutdown();

            screen_width_ = int(LOWORD(lparam));
            screen_height_ = int(HIWORD(lparam));
            env_->screen_width_ = screen_width_;
            env_->screen_height_ = screen_height_;
            env_->aspect_ =
                (float)env_->screen_width_ / (float)env_->screen_height_;

            Direct3D::GetInstance().back_buffer_RTV().Reset();
            Direct3D::GetInstance().swap_chain()->ResizeBuffers(
                0, (UINT)LOWORD(lparam), (UINT)HIWORD(lparam),
                DXGI_FORMAT_UNKNOWN, 0);

            Direct3D::GetInstance().CreateBuffer(env_.get());

            Direct3D::GetInstance().SetViewPort(0.0f, 0.0f,
                                                (float)env_->screen_width_,
                                                (float)env_->screen_height_);

            imgui_->Initialize(main_window_, env_.get());
        }

        break;
    }*/
    default: {
        return DefWindowProc(main_window, umsg, wparam, lparam);
    }
    }
}
} // namespace platform