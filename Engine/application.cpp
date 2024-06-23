#include "application.h"
#include "behavior_tree_builder.h"

using namespace Engine;

Application::Application() : imgui_(0), manager_(0), env_(0) {
    input_ = std::make_unique<Input>();
    manager_ = std::make_shared<PipelineManager>();
    env_ = std::make_shared<Env>();
    message_receiver_ = std::make_unique<MessageReceiver>();
    imgui_ = std::make_shared<Panel>(manager_);
};

bool Application::OnStart() {

    Platform::OnStart();

    env_->screen_width_ = screen_width_;
    env_->screen_height_ = screen_height_;

    std::map<EnumDataBlockType, IDataBlock *> dataBlock = {
        {EnumDataBlockType::eManager, manager_.get()},
        {EnumDataBlockType::eGui, imgui_.get()},
    };

    Direct3D::GetInstance().Initialize(env_.get(), VSYNC_ENABLED, main_window_,
                                       FULL_SCREEN);

    // clang-format off
    auto tree = new BehaviorTreeBuilder();
    tree->Build(dataBlock)
        ->Sequence()
            ->Excute(std::make_shared<InitializeCamera>())
            ->Excute(std::make_shared<InitializeCubeMapShader>())
            ->Excute(std::make_shared<InitializePhongShader>())
            ->Excute(std::make_shared<InitializeImageBasedShader>())
        ->Close()
    ->Run();
    // clang-format on

    input_->Initialize(hinstance_, main_window_, screen_width_, screen_height_);
    imgui_->Initialize(main_window_, env_.get());

    OnFrame();

    return true;
}

bool Application::OnFrame() {

    std::map<EnumDataBlockType, IDataBlock *> dataBlock = {
        {EnumDataBlockType::eManager, manager_.get()},
        {EnumDataBlockType::eEnv, env_.get()},
        {EnumDataBlockType::eGui, imgui_.get()},
    };

    // Clear the buffers to begin the scene.
    Direct3D::GetInstance().BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // clang-format off
    auto tree = std::make_unique<BehaviorTreeBuilder>();
    tree->Build(dataBlock)
    ->Excute(std::make_shared<UpdateCamera>())
    ->Conditional(std::make_shared<CheckImageBasedShader>())
        ->Sequence()
            ->Excute(std::make_shared<UpdateGameObjectsUsingImageBasedShader>())
            ->Excute(std::make_shared<RenderGameObjectsUsingImageBasedShader>())
        ->Close()
    ->End()
    ->Conditional(std::make_shared<CheckImagePhongShader>())
        ->Sequence()
            ->Excute(std::make_shared<UpdateGameObjectsUsingPhongShader>())
            ->Excute(std::make_shared<RenderGameObjectsUsingPhongShader>())
        ->Close()
    ->End()
    ->Conditional(std::make_shared<CheckImageCubeMapShader>())
        ->Sequence()
            ->Excute(std::make_shared<UpdateCubeMap>())
            ->Excute(std::make_shared<RenderCubeMap>())
        ->Close()
    ->End()
    ->Run();
    // clang-format on

    input_->Frame();
    imgui_->Frame();

    // Present the rendered scene to the screen.
    Direct3D::GetInstance().EndScene();

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

    if (env_) {
        env_.reset();
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
    auto view_port = Direct3D::GetInstance().viewport();
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
    // case WM_SIZE: {
    //     if (Direct3D::GetInstance().swap_chain()) {

    //        screen_width_ = int(LOWORD(lparam));
    //        screen_height_ = int(HIWORD(lparam));
    //        env_->screen_width_ = screen_width_;
    //        env_->screen_height_ = screen_height_;

    //        auto render = *Direct3D::GetInstance().render_target_view();
    //        render->Release();

    //        Direct3D::GetInstance().swap_chain()->ResizeBuffers(
    //            0, (UINT)LOWORD(lparam), (UINT)HIWORD(lparam),
    //            DXGI_FORMAT_UNKNOWN, 0);
    //        Direct3D::GetInstance().CreateRenderTargetView();
    //        Direct3D::GetInstance().CreateDepthBuffer(env_.get());

    //        if (ImGui::GetCurrentWindow()) {
    //            Direct3D::GetInstance().SetViewPort(
    //                ImGui::GetWindowSize().x, 0.0f,
    //                (float)env_->screen_width_ - ImGui::GetWindowSize().x,
    //                (float)env_->screen_height_);
    //            env_->aspect_ =
    //                ((float)env_->screen_width_ - ImGui::GetWindowSize().x) /
    //                (float)env_->screen_height_;
    //        }
    //    }

    //    break;
    //}
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
    default: {
        return DefWindowProc(main_window, umsg, wparam, lparam);
    }
    }
}
