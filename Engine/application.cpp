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

    auto mesh_data = GeometryGenerator::MakeBox(40.0f);
    std::reverse(mesh_data.indices.begin(), mesh_data.indices.end());
    auto cube_map = std::make_shared<Model>(
        GraphicsManager::Instance().device,
        GraphicsManager::Instance().device_context, std::vector{mesh_data});

    manager_->skybox = cube_map;

    auto envFilename = L"./Assets/Textures/Cubemaps/HDRI/SampleEnvHDR.dds";
    auto specularFilename =
        L"./Assets/Textures/Cubemaps/HDRI/SampleSpecularHDR.dds";
    auto irradianceFilename =
        L"./Assets/Textures/Cubemaps/HDRI/SampleDiffuseHDR.dds";
    auto brdfFilename = L"./Assets/Textures/Cubemaps/HDRI/SampleBrdf.dds";

    GraphicsUtil::CreateDDSTexture(GraphicsManager::Instance().device,
                                   envFilename, true, manager_->m_envSRV);
    GraphicsUtil::CreateDDSTexture(GraphicsManager::Instance().device,
                                   specularFilename, true,
                                   manager_->m_specularSRV);
    GraphicsUtil::CreateDDSTexture(GraphicsManager::Instance().device,
                                   irradianceFilename, true,
                                   manager_->m_irradianceSRV);
    GraphicsUtil::CreateDDSTexture(GraphicsManager::Instance().device,
                                   brdfFilename, true, manager_->m_brdfSRV);

    // 후처리용 화면 사각형
    {
        MeshData meshData = GeometryGenerator::MakeSquare();
        manager_->m_screenSquare =
            std::make_shared<Model>(device, context, std::vector{meshData});
    }

    // 추가 물체1
    {
        MeshData mesh = GeometryGenerator::MakeSphere(0.2f, 200, 200);
        Vector3 center(0.5f, 0.5f, 2.0f);
        auto m_obj =
            std::make_shared<Model>(device, context, std::vector{mesh});
        m_obj->UpdateWorldRow(Matrix::CreateTranslation(center));
        m_obj->m_materialConstsCPU.albedoFactor = Vector3(0.1f, 0.1f, 1.0f);
        m_obj->m_materialConstsCPU.roughnessFactor = 0.2f;
        m_obj->m_materialConstsCPU.metallicFactor = 0.6f;
        m_obj->m_materialConstsCPU.emissionFactor = Vector3(0.0f);
        m_obj->UpdateConstantBuffers(device, context);

        manager_->m_basicList.push_back(m_obj);
    }

    GraphicsUtil::CreateConstBuffer(GraphicsManager::Instance().device,
                                    manager_->m_globalConstsCPU,
                                    manager_->m_globalConstsGPU);

    GraphicsUtil::CreateConstBuffer(GraphicsManager::Instance().device,
                                    manager_->m_reflectGlobalConstsCPU,
                                    manager_->m_reflectGlobalConstsGPU);

    // 그림자맵 렌더링할 때 사용할 GlobalConsts들 별도 생성
    for (int i = 0; i < MAX_LIGHTS; i++) {
        GraphicsUtil::CreateConstBuffer(GraphicsManager::Instance().device,
                                        manager_->m_shadowGlobalConstsCPU[i],
                                        manager_->m_shadowGlobalConstsGPU[i]);
    }

    // 후처리 효과용 ConstBuffer
    GraphicsUtil::CreateConstBuffer(GraphicsManager::Instance().device,
                                    manager_->m_postEffectsConstsCPU,
                                    manager_->m_postEffectsConstsGPU);

    manager_->m_postProcess.Initialize(
        device, context, {GraphicsManager::Instance().postEffectsSRV},
        {GraphicsManager::Instance().back_buffer_RTV},
        common::Env::Instance().screen_width,
        common::Env::Instance().screen_height, 4);

    // clang-format off
    auto tree = new BehaviorTreeBuilder();
    tree->Build(dataBlock)
        ->Sequence()
            //->Excute(std::make_shared<InitializeBoardMap>())
            ->Excute(std::make_shared<InitializeCamera>())
            //->Excute(std::make_shared<InitializeCubeMapShader>())
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
        GraphicsManager::Instance().m_depthStencilView.Get());

    context->ClearDepthStencilView(
        GraphicsManager::Instance().m_depthStencilView.Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Clear the buffers to begin the scene.
    // GraphicsManager::Instance().BeginScene(imgui_->Tab().common.draw_as_wire_);

    // clang-format off

    if (imgui_->SelectedId()) {
        imgui_->PushNode(manager_->models[imgui_->SelectedId()]); 
    }

    //auto tree = std::make_unique<BehaviorTreeBuilder>();
    //tree->Build(dataBlock)
    //->Excute(std::make_shared<UpdateCamera>())
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
    //->Run();
    // clang-format on

    // 그림자 렌더링에 사용
    // manager_->m_globalConstsCPU.invViewProj =
    // manager_->m_globalConstsCPU.viewProj.Invert();


    auto eyeWorld = manager_->camera->GetEyePos();
    auto view = manager_->camera->GetViewRow();
    auto proj = manager_->camera->GetProjRow();

    manager_->m_globalConstsCPU.eyeWorld = eyeWorld;
    manager_->m_globalConstsCPU.view = view.Transpose();
    manager_->m_globalConstsCPU.proj = proj.Transpose();
    manager_->m_globalConstsCPU.invProj = proj.Invert().Transpose();
    manager_->m_globalConstsCPU.viewProj = (view * proj).Transpose();
    manager_->m_globalConstsCPU.invViewProj = view.Invert();

    manager_->m_globalConstsCPU.textureToDraw = 0;
    manager_->m_globalConstsCPU.envLodBias = 0.1f;
    manager_->m_globalConstsCPU.lodBias = 2.1f;

    GraphicsUtil::UpdateBuffer(device, context, manager_->m_globalConstsCPU,
                               manager_->m_globalConstsGPU);

    GraphicsManager::Instance().SetGlobalConsts(manager_->m_globalConstsGPU);

    GraphicsManager::Instance().SetPipelineState(Graphics::defaultSolidPSO);

    for (auto &i : manager_->m_basicList) {
        i->Render(context);
    }

    GraphicsManager::Instance().SetGlobalConsts(manager_->m_globalConstsGPU);
    GraphicsManager::Instance().SetPipelineState(Graphics::skyboxSolidPSO);

    auto cube = dynamic_cast<Model *>(manager_->skybox.get());
    cube->Render(context);

    context->ResolveSubresource(
        GraphicsManager::Instance().resolved_buffer.Get(), 0,
        GraphicsManager::Instance().float_buffer.Get(), 0,
        DXGI_FORMAT_R16G16B16A16_FLOAT);

    // PostEffects
    GraphicsManager::Instance().SetPipelineState(Graphics::postEffectsPSO);

    std::vector<ID3D11ShaderResourceView *> postEffectsSRVs = {
        GraphicsManager::Instance().resolved_SRV.Get(), nullptr};

    // 그림자맵 확인용 임시
    // AppBase::SetGlobalConsts(m_shadowGlobalConstsGPU[0]);
    GraphicsManager::Instance().SetGlobalConsts(manager_->m_globalConstsGPU);
    // vector<ID3D11ShaderResourceView *> postEffectsSRVs = {
    //  m_resolvedSRV.Get(), m_shadowSRVs[1].Get()};

    // 20번에 넣어줌
    context->PSSetShaderResources(20, UINT(postEffectsSRVs.size()),
                                  postEffectsSRVs.data());
    context->OMSetRenderTargets(
        1, GraphicsManager::Instance().postEffectsRTV.GetAddressOf(), NULL);
    // m_context->OMSetRenderTargets(1, m_backBufferRTV.GetAddressOf(), NULL);

    context->PSSetConstantBuffers(
        3, 1, manager_->m_postEffectsConstsGPU.GetAddressOf());
    manager_->m_screenSquare->Render(context);

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
    if (common::Env::Instance().vsync_enabled) {
        GraphicsManager::Instance() .swap_chain->Present(1, 0);
    }
    else {
        GraphicsManager::Instance().swap_chain->Present(0, 0);
    }

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

        if (ImGui::GetCurrentWindow()) {

            auto cursor = ImGui::GetMousePos();

            auto left = GraphicsManager::Instance().viewport.TopLeftX +
                        ImGui::GetWindowSize().x;
            auto top = GraphicsManager::Instance().viewport.TopLeftY;
            auto right = GraphicsManager::Instance().viewport.Width + left;
            auto bottom = GraphicsManager::Instance().viewport.Height + top;

            if (left < cursor.x && cursor.x < right && top < cursor.y &&
                cursor.y < bottom)
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
            //if (CheckIfMouseInViewport()) 
            {
                return message_receiver_->OnMouseRightDragRequest(manager_.get(),
                                                             input_);
            }
        }
        if (wparam & MK_MBUTTON) {
            //if (CheckIfMouseInViewport()) 
            {
                return message_receiver_->OnMouseWheelDragRequest(
                    manager_.get(), input_, LOWORD(lparam), HIWORD(lparam));
            }
        }
        break;
    }
    case WM_MOUSEWHEEL: {
        //if (CheckIfMouseInViewport()) 
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

            GraphicsManager::Instance().SetViewPort(
                0.0f, 0.0f, (float)common::Env::Instance().screen_width,
                (float)common::Env::Instance().screen_height);

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