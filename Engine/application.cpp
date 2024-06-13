#include "Application.h"

using namespace Engine;

bool Application::OnStart() {

    Platform::OnStart();

    manager_= new PipelineManager();
    env_= new Env();
    imgui_= new ImGuiManager();

    env_->screen_width_ = screen_width_;
    env_->screen_height_ = screen_height_;

    // Create and initialize the input object.  This object will be used to
    // handle reading the keyboard input from the user.
    input_= std::make_unique<Input>();

    input_->Initialize(hinstance_, main_window_, screen_width_, screen_height_);

    std::map<EnumDataBlockType, IDataBlock *> dataBlock = {
        {EnumDataBlockType::eManager, manager_},
        {EnumDataBlockType::eGui, imgui_},
    };

    Direct3D::GetInstance().Init(env_, VSYNC_ENABLED, main_window_,
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
        {EnumDataBlockType::eManager, manager_},
        {EnumDataBlockType::eEnv, env_},
        {EnumDataBlockType::eGui, imgui_},
    };

    // Clear the buffers to begin the scene.
    Direct3D::GetInstance().BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    if (!input_->Frame()) {
        return false;
    }
    // ImGui
    imgui_->Prepare(env_);

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
        delete env_;
        env_= 0;
    }

    if (imgui_) {
        imgui_->Shutdown();
        delete imgui_;
        imgui_= 0;
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
            return message_receiver_->OnRightDragRequest(manager_, input_);
        }
        break;
    }
    case WM_MOUSEWHEEL: {
        return message_receiver_->OnMouseWheelRequest(manager_, input_);
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
        return message_receiver_->OnModelLoadRequest(manager_, main_window);
        break;
    }
    case WM_SPHERE_LOAD: {
        return message_receiver_->OnSphereLoadRequest(manager_);
        break;
    }
    case WM_BOX_LOAD: {
        return message_receiver_->OnBoxLoadRequest(manager_);
        break;
    }
    case WM_CYLINDER_LOAD: {
        return message_receiver_->OnCylinderLoadRequest(manager_);
        break;
    }
    default: {
        return DefWindowProc(main_window, umsg, wparam, lparam);
    }
    }
}
