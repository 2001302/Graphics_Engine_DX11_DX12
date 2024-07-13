#include "application.h"
#include "behavior_tree_builder.h"

namespace platform {

Application::Application() : imgui_(0), manager_(0) {
    input_ = std::make_unique<dx11::Input>();
    manager_ = std::make_shared<dx11::PipelineManager>();
    message_receiver_ = std::make_unique<dx11::MessageReceiver>();
    imgui_ = std::make_shared<common::SettingUi>();
};

bool Application::OnStart() {

    Platform::OnStart();

    std::map<dx11::EnumDataBlockType, common::IDataBlock *> dataBlock = {
        {dx11::EnumDataBlockType::eManager, manager_.get()},
        {dx11::EnumDataBlockType::eGui, imgui_.get()},
    };

    dx11::GraphicsContext::Instance().Initialize();

    input_->Initialize(hinstance_, common::Env::Instance().screen_width,
                       common::Env::Instance().screen_height);
    imgui_->Initialize();

    // clang-format off
    auto tree = new dx11::BehaviorTreeBuilder();
    tree->Build(dataBlock)
        ->Sequence()
            ->Excute(std::make_shared<dx11::InitializeBoardMap>())
            ->Excute(std::make_shared<dx11::InitializeCamera>())
            ->Excute(std::make_shared<dx11::InitializeGroundShader>())
            ->Excute(std::make_shared<dx11::InitializeCubeMapShader>())
            ->Excute(std::make_shared<dx11::InitializePhongShader>())
            ->Excute(std::make_shared<dx11::InitializeImageBasedShader>())
            ->Excute(std::make_shared<dx11::InitializePhysicallyBasedShader>())
        ->Close()
    ->Run();
    // clang-format on

    OnFrame();

    return true;
}

bool Application::OnFrame() {

    std::map<dx11::EnumDataBlockType, common::IDataBlock *> dataBlock = {
        {dx11::EnumDataBlockType::eManager, manager_.get()},
        {dx11::EnumDataBlockType::eGui, imgui_.get()},
    };

    // Clear the buffers to begin the scene.
    dx11::GraphicsContext::Instance().BeginScene(
        0.0f, 0.0f, 0.0f, 1.0f, imgui_->Tab().common.draw_as_wire_);

    // clang-format off
    std::vector<int> model_ids;
    for(auto model : manager_->models)
        model_ids.push_back(model.first);
    
    auto tree = std::make_unique<dx11::BehaviorTreeBuilder>();
    tree->Build(dataBlock)
    ->Excute(std::make_shared<dx11::UpdateCamera>())
    ->Parallel(model_ids)
        ->Conditional(std::make_shared<dx11::CheckImageBasedShader>())
            ->Sequence()
                ->Excute(std::make_shared<dx11::UpdateGameObjectsUsingImageBasedShader>())
                ->Excute(std::make_shared<dx11::RenderGameObjectsUsingImageBasedShader>())
            ->Close()
        ->End()
        ->Conditional(std::make_shared<dx11::CheckPhongShader>())
            ->Sequence()
                ->Excute(std::make_shared<dx11::UpdateGameObjectsUsingPhongShader>())
                ->Excute(std::make_shared<dx11::RenderGameObjectsUsingPhongShader>())
            ->Close()
        ->End()
        ->Conditional(std::make_shared<dx11::CheckPhysicallyBasedShader>())
            ->Sequence()
                ->Excute(std::make_shared<dx11::UpdateGameObjectsUsingPhysicallyBasedShader>())
                ->Excute(std::make_shared<dx11::RenderGameObjectsUsingPhysicallyBasedShader>())
            ->Close()
        ->End()
    ->Close()
    ->Conditional(std::make_shared<dx11::CheckGroundShader>())
        ->Sequence()
            ->Excute(std::make_shared<dx11::UpdateGroundShader>())
            ->Excute(std::make_shared<dx11::RenderGroundShader>())
        ->Close()
    ->End()
    ->Conditional(std::make_shared<dx11::CheckCubeMapShader>())
        ->Sequence()
            ->Excute(std::make_shared<dx11::UpdateCubeMap>())
            ->Excute(std::make_shared<dx11::RenderCubeMap>())
        ->Close()
    ->End()
    ->Excute(std::make_shared<dx11::RenderBoardMap>())
    ->Run();
    // clang-format on

    input_->Frame();
    imgui_->Frame(manager_->models);

    // Present the rendered scene to the screen.
    dx11::GraphicsContext::Instance().EndScene();

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
    auto view_port = dx11::GraphicsContext::Instance().viewport();
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