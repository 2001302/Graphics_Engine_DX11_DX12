#include "Application.h"

using namespace Engine;

Application::Application() : imgui_(0), manager_(0), env_(0) {
    input_ = std::make_unique<Input>();
    imgui_ = std::make_shared<ImGuiManager>();
    manager_ = std::make_shared<PipelineManager>();
    env_ = std::make_shared<Env>();
    message_receiver_ = std::make_unique<MessageReceiver>();
};

bool Application::OnStart() {

    Platform::OnStart();

    env_->screen_width_ = screen_width_;
    env_->screen_height_ = screen_height_;

    input_->Initialize(hinstance_, main_window_, screen_width_, screen_height_);

    std::map<EnumDataBlockType, IDataBlock *> dataBlock = {
        {EnumDataBlockType::eManager, manager_.get()},
        {EnumDataBlockType::eGui, imgui_.get()},
    };

    Direct3D::GetInstance().Init(env_.get(), VSYNC_ENABLED, main_window_,
                                 FULL_SCREEN);

    auto tree = new BehaviorTreeBuilder();

    tree->Build(dataBlock)
        ->Sequence()
        ->Excute(std::make_shared<InitializeCamera>())
        ->Excute(std::make_shared<InitializePhongShader>(main_window_))
        ->Close();

    tree->Run();

    imgui_->Initialize(main_window_, &Direct3D::GetInstance());

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

    if (!input_->Frame()) {
        return false;
    }
    // ImGui
    imgui_->Prepare(env_.get());

    auto tree = std::make_unique<BehaviorTreeBuilder>();

    tree->Build(dataBlock)
        ->Sequence()
        ->Excute(std::make_shared<RenderGameObjects>())
        ->Close();

    tree->Run();

    imgui_->Render(main_window_);

    // Present the rendered scene to the screen.
    Direct3D::GetInstance().EndScene();

    return true;
}

bool Application::OnStop() {
    Platform::OnStop();

    if (manager_) {
        for (auto &model : manager_->models) {
            delete model;
            model = 0;
        }

        manager_->phongShader.reset();
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
LRESULT CALLBACK Application::MessageHandler(HWND main_window, UINT umsg,
                                             WPARAM wparam, LPARAM lparam) {
    extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
        HWND main_window, UINT msg, WPARAM wParam, LPARAM lParam);

    ImGui_ImplWin32_WndProcHandler(main_window, umsg, wparam, lparam);

    switch (umsg) {
    case WM_MOUSEMOVE: {
        if (wparam & MK_RBUTTON) {
            return message_receiver_->OnRightDragRequest(manager_.get(),
                                                         input_);
        }
        break;
    }
    case WM_MOUSEWHEEL: {
        return message_receiver_->OnMouseWheelRequest(manager_.get(), input_);
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
