#include "application.h"
#include "behavior_tree_builder.h"

using namespace Engine;

Application::Application() : imgui_(0), manager_(0), env_(0) {
    input_ = std::make_unique<Input>();
    manager_ = std::make_shared<PipelineManager>();
    env_ = std::make_shared<Env>();
    message_receiver_ = std::make_unique<MessageReceiver>();
    imgui_ = std::make_shared<Panel>(manager_);
    post_process_ = std::make_unique<PostProcess>();
};

bool Application::OnStart() {

    Platform::OnStart();

    env_->screen_width_ = screen_width_;
    env_->screen_height_ = screen_height_;

    std::map<EnumDataBlockType, IDataBlock *> dataBlock = {
        {EnumDataBlockType::eManager, manager_.get()},
        {EnumDataBlockType::eGui, imgui_.get()},
    };

    GeometryGenerator::MakeSquare(post_process_.get());
    Direct3D::GetInstance().Initialize(env_.get(), VSYNC_ENABLED, main_window_,
                                       FULL_SCREEN);

    auto device = Direct3D::GetInstance().device();
    auto context = Direct3D::GetInstance().device_context();

    post_process_->Initialize(device, context,
                              {Direct3D::GetInstance().resolved_SRV()},
                              {Direct3D::GetInstance().back_buffer_RTV()},
                              screen_width_, screen_height_, 4);

    input_->Initialize(hinstance_, main_window_, screen_width_, screen_height_);
    imgui_->Initialize(main_window_, env_.get());

    // clang-format off
    auto tree = new BehaviorTreeBuilder();
    tree->Build(dataBlock)
        ->Sequence()
            ->Excute(std::make_shared<InitializeCamera>())
            ->Excute(std::make_shared<InitializeCubeMapShader>())
            ->Excute(std::make_shared<InitializePhongShader>())
            ->Excute(std::make_shared<InitializeImageBasedShader>())
            ->Excute(std::make_shared<InitializePhysicallyBasedShader>())
        ->Close()
    ->Run();
    // clang-format on

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
            ->Excute(std::make_shared<UpdateCubeMap>())
            ->Excute(std::make_shared<RenderCubeMap>())
        ->Close()
    ->End()
    ->Conditional(std::make_shared<CheckPhongShader>())
        ->Sequence()
            ->Excute(std::make_shared<UpdateGameObjectsUsingPhongShader>())
            ->Excute(std::make_shared<RenderGameObjectsUsingPhongShader>())
        ->Close()
    ->End()
    ->Conditional(std::make_shared<CheckPhysicallyBasedShader>())
        ->Sequence()
            ->Excute(std::make_shared<UpdateGameObjectsUsingPhysicallyBasedShader>())
            ->Excute(std::make_shared<RenderGameObjectsUsingPhysicallyBasedShader>())
            ->Excute(std::make_shared<UpdateCubeMap>())
            ->Excute(std::make_shared<RenderCubeMap>())
        ->Close()
    ->End()
    ->Run();
    // clang-format on

    Direct3D::GetInstance().device_context()->ResolveSubresource(
        Direct3D::GetInstance().resolved_buffer().Get(), 0,
        Direct3D::GetInstance().float_buffer().Get(), 0,
        DXGI_FORMAT_R16G16B16A16_FLOAT);

    auto context = Direct3D::GetInstance().device_context();
    post_process_->Render(context);

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
