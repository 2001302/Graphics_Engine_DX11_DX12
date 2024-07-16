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
    
    if (imgui_->SelectedId()) {
        imgui_->PushNode(manager_->models[imgui_->SelectedId()].get()); 
    }

    auto tree = std::make_unique<dx11::BehaviorTreeBuilder>();
    tree->Build(dataBlock)
    ->Excute(std::make_shared<dx11::UpdateCamera>())
    ->Parallel(model_ids)
        ->Sequence()
            ->Excute(std::make_shared<dx11::UpdateGameObjectsUsingPhysicallyBasedShader>())
            ->Excute(std::make_shared<dx11::RenderGameObjectsUsingPhysicallyBasedShader>())
        ->Close()
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

    imgui_->FrameBegin();
    imgui_->FrameRate();
    imgui_->StyleSetting();
    imgui_->MenuBar();
    imgui_->NodeEditor();
    imgui_->TabBar(manager_->models);
    imgui_->FrameEnd();
    imgui_->ClearNode();

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
            if (wparam & MK_MBUTTON) {
                return message_receiver_->OnWheelDragRequest(
                    manager_.get(), input_, LOWORD(lparam), HIWORD(lparam));
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
    case WM_SIZE: {
        if (dx11::GraphicsContext::Instance().swap_chain()) {

            imgui_->Shutdown();

            common::Env::Instance().screen_width = int(LOWORD(lparam));
            common::Env::Instance().screen_height = int(HIWORD(lparam));
            common::Env::Instance().aspect =
                (float)common::Env::Instance().screen_width /
                (float)common::Env::Instance().screen_height;

            dx11::GraphicsContext::Instance().back_buffer_RTV().Reset();
            dx11::GraphicsContext::Instance().swap_chain()->ResizeBuffers(
                0, (UINT)LOWORD(lparam), (UINT)HIWORD(lparam),
                DXGI_FORMAT_UNKNOWN, 0);

            dx11::GraphicsContext::Instance().CreateBuffer();

            dx11::GraphicsContext::Instance().SetViewPort(
                0.0f, 0.0f, (float)common::Env::Instance().screen_width,
                (float)common::Env::Instance().screen_height);

            imgui_->Initialize();
        }

        break;
    }
    default: {
        return DefWindowProc(main_window, umsg, wparam, lparam);
    }
    }
}
} // namespace platform