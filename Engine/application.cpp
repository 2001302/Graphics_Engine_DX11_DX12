#include "application.h"
#include "behavior_tree_builder.h"

namespace engine {

Application::Application() {
    manager_ = std::make_shared<PipelineManager>();
    message_receiver_ = std::make_unique<MessageReceiver>();
    input_ = std::make_unique<Input>();
    imgui_ = std::make_shared<common::SettingUi>();
};

bool Application::OnStart() {

    Platform::OnStart();
    input_->Initialize(hinstance_);
    imgui_->Initialize();

    std::map<EnumDataBlockType, common::IDataBlock *> dataBlock = {
        {EnumDataBlockType::eManager, manager_.get()},
        {EnumDataBlockType::eGui, imgui_.get()},
    };

    auto device = GraphicsManager::Instance().device;
    auto context = GraphicsManager::Instance().device_context;

    manager_->m_postProcess.Initialize(
        device, context, {GraphicsManager::Instance().postEffectsSRV},
        {GraphicsManager::Instance().back_buffer_RTV},
        common::Env::Instance().screen_width,
        common::Env::Instance().screen_height, 4);

    GraphicsUtil::CreateConstBuffer(GraphicsManager::Instance().device,
                                    manager_->m_globalConstsCPU,
                                    manager_->m_globalConstsGPU);

    // clang-format off
    auto tree = new BehaviorTreeBuilder();
    tree->Build(dataBlock)
        ->Sequence()
            //->Excute(std::make_shared<InitializeBoardMap>())
            ->Excute(std::make_shared<InitializeCamera>())
            ->Excute(std::make_shared<InitializeCubeMapShader>())
            //->Excute(std::make_shared<InitializePhongShader>())
            //->Excute(std::make_shared<InitializePhysicallyBasedShader>())
        ->Close()
    ->Run();
    // clang-format on

    OnFrame();

    return true;
}

bool Application::OnFrame() {

    std::map<EnumDataBlockType, common::IDataBlock *> dataBlock = {
        {EnumDataBlockType::eManager, manager_.get()},
        {EnumDataBlockType::eGui, imgui_.get()},
    };

    auto device = GraphicsManager::Instance().device;
    auto context = GraphicsManager::Instance().device_context;

    context->VSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                           Graphics::sampleStates.data());
    context->PSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                           Graphics::sampleStates.data());

    // 공용 텍스춰들: "Common.hlsli"에서 register(t10)부터 시작
    std::vector<ID3D11ShaderResourceView *> commonSRVs = {
        manager_->m_envSRV.Get(), manager_->m_specularSRV.Get(),
        manager_->m_irradianceSRV.Get(), manager_->m_brdfSRV.Get()};
    context->PSSetShaderResources(10, UINT(commonSRVs.size()),
                                  commonSRVs.data());

    const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    std::vector<ID3D11RenderTargetView *> rtvs = {
        GraphicsManager::Instance().float_RTV.Get()};

    for (size_t i = 0; i < rtvs.size(); i++) {
        context->ClearRenderTargetView(rtvs[i], clearColor);
    }
    context->OMSetRenderTargets(
        UINT(rtvs.size()), rtvs.data(),
        GraphicsManager::Instance().depth_stencil_view.Get());

    context->ClearDepthStencilView(
        GraphicsManager::Instance().depth_stencil_view.Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    GraphicsManager::Instance().SetGlobalConsts(manager_->m_globalConstsGPU);

    // Clear the buffers to begin the scene.
    // GraphicsManager::Instance().BeginScene(imgui_->Tab().common.draw_as_wire_);

    // clang-format off

    if (imgui_->SelectedId()) {
        imgui_->PushNode(manager_->models[imgui_->SelectedId()]); 
    }

    auto tree = std::make_unique<BehaviorTreeBuilder>();
    tree->Build(dataBlock)
    ->Excute(std::make_shared<UpdateCamera>())
    //->Parallel(manager_->models)
    //    ->Selector()
    //        ->Sequence()
    //            ->Excute(std::make_shared<CheckPhongShader>())
    //            ->Excute(std::make_shared<UpdateGameObjectsUsingPhongShader>())
    //            ->Excute(std::make_shared<RenderGameObjectsUsingPhongShader>())
    //        ->Close()
    //        ->Sequence()
    //            ->Excute(std::make_shared<CheckPhysicallyBasedShader>())
    //            ->Excute(std::make_shared<UpdateGameObjectsUsingPhysicallyBasedShader>())
    //            ->Excute(std::make_shared<RenderGameObjectsUsingPhysicallyBasedShader>())
    //        ->Close()
    //    ->Close()
    //->Close()
    //->Conditional(std::make_shared<CheckCubeMapShader>())
    //    ->Sequence()
    //        ->Excute(std::make_shared<UpdateCubeMap>())
    //        ->Excute(std::make_shared<RenderCubeMap>())
    //    ->Close()
    //->End()
    //->Excute(std::make_shared<RenderBoardMap>())
    ->Run();
    // clang-format on

    context->ResolveSubresource(
        GraphicsManager::Instance().resolved_buffer.Get(), 0,
        GraphicsManager::Instance().float_buffer.Get(), 0,
        DXGI_FORMAT_R16G16B16A16_FLOAT);

    GraphicsManager::Instance().SetPipelineState(Graphics::postProcessingPSO);
    manager_->m_postProcess.Render(context);

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
    GraphicsManager::Instance().EndScene();

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
    auto cursor = ImGui::GetMousePos();
    auto view_port = GraphicsManager::Instance().viewport;
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
        if (GraphicsManager::Instance().swap_chain) {

            imgui_->Shutdown();

            common::Env::Instance().screen_width = int(LOWORD(lparam));
            common::Env::Instance().screen_height = int(HIWORD(lparam));
            common::Env::Instance().aspect =
                (float)common::Env::Instance().screen_width /
                (float)common::Env::Instance().screen_height;

            GraphicsManager::Instance().back_buffer_RTV.Reset();
            GraphicsManager::Instance().swap_chain->ResizeBuffers(
                0, (UINT)LOWORD(lparam), (UINT)HIWORD(lparam),
                DXGI_FORMAT_UNKNOWN, 0);

            GraphicsManager::Instance().CreateBuffer();

            GraphicsManager::Instance().SetViewPort(
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
} // namespace engine