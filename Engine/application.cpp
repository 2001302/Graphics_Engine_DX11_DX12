#include "application.h"
#include "behavior_tree_builder.h"
#include "renderer_initialize_node.h"
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

void Application::UpdateGlobalConstants(const Vector3 &eyeWorld,
                                        const Matrix &viewRow,
                                        const Matrix &projRow,
                                        const Matrix &refl = Matrix()) {

    auto device = GraphicsManager::Instance().device;
    auto context = GraphicsManager::Instance().device_context;

    manager_->m_globalConstsCPU.eyeWorld = eyeWorld;
    manager_->m_globalConstsCPU.view = viewRow.Transpose();
    manager_->m_globalConstsCPU.proj = projRow.Transpose();
    manager_->m_globalConstsCPU.invProj = projRow.Invert().Transpose();
    manager_->m_globalConstsCPU.viewProj = (viewRow * projRow).Transpose();
    // 그림자 렌더링에 사용
    manager_->m_globalConstsCPU.invViewProj =
        manager_->m_globalConstsCPU.viewProj.Invert();

    manager_->m_reflectGlobalConstsCPU = manager_->m_globalConstsCPU;
    memcpy(&manager_->m_reflectGlobalConstsCPU, &manager_->m_globalConstsCPU,
           sizeof(manager_->m_globalConstsCPU));
    manager_->m_reflectGlobalConstsCPU.view = (refl * viewRow).Transpose();
    manager_->m_reflectGlobalConstsCPU.viewProj =
        (refl * viewRow * projRow).Transpose();
    // 그림자 렌더링에 사용 (TODO: 광원의 위치도 반사시킨 후에 계산해야 함)
    manager_->m_reflectGlobalConstsCPU.invViewProj =
        manager_->m_reflectGlobalConstsCPU.viewProj.Invert();

    GraphicsUtil::UpdateBuffer(device, context, manager_->m_globalConstsCPU,
                               manager_->m_globalConstsGPU);
    GraphicsUtil::UpdateBuffer(device, context,
                               manager_->m_reflectGlobalConstsCPU,
                               manager_->m_reflectGlobalConstsGPU);
}

bool Application::OnUpdate(float dt) {
    auto device = GraphicsManager::Instance().device;
    auto context = GraphicsManager::Instance().device_context;

    manager_->camera->Update();
    // 반사 행렬 추가
    const Vector3 eyeWorld = manager_->camera->GetPosition();
    const Matrix reflectRow = Matrix::CreateReflection(manager_->m_mirrorPlane);
    const Matrix viewRow = manager_->camera->GetView();
    const Matrix projRow = manager_->camera->GetProjection();

    UpdateLights(dt);

    // 공용 ConstantBuffer 업데이트
    UpdateGlobalConstants(eyeWorld, viewRow, projRow, reflectRow);

    // 거울은 따로 처리
    if (true)
    {
        Renderer *renderer = nullptr;
        manager_->m_mirror->GetComponent(EnumComponentType::eRenderer,
            (Component **)(&renderer));
        renderer->UpdateConstantBuffers(device, context);
    }

    // 조명의 위치 반영
    for (int i = 0; i < MAX_LIGHTS; i++) {
        Renderer *renderer = nullptr;
        manager_->m_lightSphere[i]->GetComponent(EnumComponentType::eRenderer,
                                                 (Component **)(&renderer));

        renderer->UpdateWorldRow(
            Matrix::CreateScale((std::max)(
                0.01f, manager_->m_globalConstsCPU.lights[i].radius)) *
            Matrix::CreateTranslation(
                manager_->m_globalConstsCPU.lights[i].position));
    }

    //// 마우스 이동/회전 반영
    // if (m_leftButton || m_rightButton) {
    //     Quaternion q;
    //     Vector3 dragTranslation;
    //     Vector3 pickPoint;
    //     if (UpdateMouseControl(m_mainBoundingSphere, q, dragTranslation,
    //                            pickPoint)) {
    //         Vector3 translation = m_mainObj->m_worldRow.Translation();
    //         m_mainObj->m_worldRow.Translation(Vector3(0.0f));
    //         m_mainObj->UpdateWorldRow(
    //             m_mainObj->m_worldRow * Matrix::CreateFromQuaternion(q) *
    //             Matrix::CreateTranslation(dragTranslation + translation));
    //         m_mainBoundingSphere.Center =
    //         m_mainObj->m_worldRow.Translation();

    //        // 충돌 지점에 작은 구 그리기
    //        m_cursorSphere->m_isVisible = true;
    //        m_cursorSphere->UpdateWorldRow(
    //            Matrix::CreateTranslation(pickPoint));
    //    } else {
    //        m_cursorSphere->m_isVisible = false;
    //    }
    //} else {
    //    m_cursorSphere->m_isVisible = false;
    //}

    for (auto &i : manager_->m_basicList) {

        Renderer *renderer = nullptr;
        i->GetComponent(EnumComponentType::eRenderer,
                        (Component **)(&renderer));
        renderer->UpdateConstantBuffers(device, context);
    }

    return true;
}
void Application::UpdateLights(float dt) {

    auto device = GraphicsManager::Instance().device;
    auto context = GraphicsManager::Instance().device_context;

    // 회전하는 lights[1] 업데이트
    static Vector3 lightDev = Vector3(1.0f, 0.0f, 0.0f);
    if (manager_->m_lightRotate) {
        lightDev = Vector3::Transform(
            lightDev, Matrix::CreateRotationY(dt * 3.141592f * 0.5f));
    }
    manager_->m_globalConstsCPU.lights[1].position =
        Vector3(0.0f, 1.1f, 2.0f) + lightDev;
    Vector3 focusPosition = Vector3(0.0f, -0.5f, 1.7f);
    manager_->m_globalConstsCPU.lights[1].direction =
        focusPosition - manager_->m_globalConstsCPU.lights[1].position;
    manager_->m_globalConstsCPU.lights[1].direction.Normalize();

    // 그림자맵을 만들기 위한 시점
    for (int i = 0; i < MAX_LIGHTS; i++) {
        const auto &light = manager_->m_globalConstsCPU.lights[i];
        if (light.type & LIGHT_SHADOW) {

            Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
            if (abs(up.Dot(light.direction) + 1.0f) < 1e-5)
                up = Vector3(1.0f, 0.0f, 0.0f);

            // 그림자맵을 만들 때 필요
            Matrix lightViewRow = DirectX::XMMatrixLookAtLH(
                light.position, light.position + light.direction, up);

            Matrix lightProjRow = DirectX::XMMatrixPerspectiveFovLH(
                DirectX::XMConvertToRadians(120.0f), 1.0f, 0.1f, 10.0f);

            manager_->m_shadowGlobalConstsCPU[i].eyeWorld = light.position;
            manager_->m_shadowGlobalConstsCPU[i].view =
                lightViewRow.Transpose();
            manager_->m_shadowGlobalConstsCPU[i].proj =
                lightProjRow.Transpose();
            manager_->m_shadowGlobalConstsCPU[i].invProj =
                lightProjRow.Invert().Transpose();
            manager_->m_shadowGlobalConstsCPU[i].viewProj =
                (lightViewRow * lightProjRow).Transpose();

            // LIGHT_FRUSTUM_WIDTH 확인
            // Vector4 eye(0.0f, 0.0f, 0.0f, 1.0f);
            // Vector4 xLeft(-1.0f, -1.0f, 0.0f, 1.0f);
            // Vector4 xRight(1.0f, 1.0f, 0.0f, 1.0f);
            // eye = Vector4::Transform(eye, lightProjRow);
            // xLeft = Vector4::Transform(xLeft, lightProjRow.Invert());
            // xRight = Vector4::Transform(xRight, lightProjRow.Invert());
            // xLeft /= xLeft.w;
            // xRight /= xRight.w;
            // cout << "LIGHT_FRUSTUM_WIDTH = " << xRight.x - xLeft.x << endl;

            GraphicsUtil::UpdateBuffer(device, context,
                                       manager_->m_shadowGlobalConstsCPU[i],
                                       manager_->m_shadowGlobalConstsGPU[i]);

            // 그림자를 실제로 렌더링할 때 필요
            manager_->m_globalConstsCPU.lights[i].viewProj =
                manager_->m_shadowGlobalConstsCPU[i].viewProj;
            manager_->m_globalConstsCPU.lights[i].invProj =
                manager_->m_shadowGlobalConstsCPU[i].invProj;

            // 반사된 장면에서도 그림자를 그리고 싶다면 조명도 반사시켜서
            // 넣어주면 됩니다.
        }
    }
}

bool Application::OnRender() {

    GraphicsManager::Instance().SetMainViewport();

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

    // Depth Only Pass (RTS 생략 가능)
    context->OMSetRenderTargets(
        0, NULL, GraphicsManager::Instance().m_depthOnlyDSV.Get());
    context->ClearDepthStencilView(
        GraphicsManager::Instance().m_depthOnlyDSV.Get(), D3D11_CLEAR_DEPTH,
        1.0f, 0);

    GraphicsManager::Instance().SetPipelineState(Graphics::depthOnlyPSO);
    GraphicsManager::Instance().SetGlobalConsts(manager_->m_globalConstsGPU);

    for (auto &i : manager_->m_basicList) {
        Renderer *renderer = nullptr;
        i->GetComponent(EnumComponentType::eRenderer,
                        (Component **)(&renderer));
        renderer->Render(context);
    }

    if (true) {
        Renderer *renderer = nullptr;
        manager_->skybox->GetComponent(EnumComponentType::eRenderer,
                                       (Component **)(&renderer));
        renderer->Render(context);
    }

    if (true) {
        Renderer *renderer = nullptr;
        manager_->m_mirror->GetComponent(EnumComponentType::eRenderer,
                                         (Component **)(&renderer));
        renderer->Render(context);
    }

    // 그림자맵 만들기
    GraphicsManager::Instance().SetShadowViewport(); // 그림자맵 해상도
    GraphicsManager::Instance().SetPipelineState(Graphics::depthOnlyPSO);
    for (int i = 0; i < MAX_LIGHTS; i++) {
        if (manager_->m_globalConstsCPU.lights[i].type & LIGHT_SHADOW) {
            // RTS 생략 가능
            context->OMSetRenderTargets(
                0, NULL, GraphicsManager::Instance().m_shadowDSVs[i].Get());
            context->ClearDepthStencilView(
                GraphicsManager::Instance().m_shadowDSVs[i].Get(),
                D3D11_CLEAR_DEPTH, 1.0f, 0);
            GraphicsManager::Instance().SetGlobalConsts(
                manager_->m_shadowGlobalConstsGPU[i]);

            for (auto &i : manager_->m_basicList) {
                Renderer *renderer = nullptr;
                i->GetComponent(EnumComponentType::eRenderer,
                                (Component **)(&renderer));

                if (renderer->m_castShadow && renderer->m_isVisible)
                    renderer->Render(context);
            }

            if (true) {
                Renderer *renderer = nullptr;
                manager_->skybox->GetComponent(
                    EnumComponentType::eRenderer,
                    (Component **)(&renderer));
                renderer->Render(context);
            }

            if (true) {
                Renderer *renderer = nullptr;
                manager_->m_mirror->GetComponent(
                    EnumComponentType::eRenderer,
                    (Component **)(&renderer));
                renderer->Render(context);
            }
        }
    }

    // 다시 렌더링 해상도로 되돌리기
    GraphicsManager::Instance().SetMainViewport();

    // 거울 1. 거울은 빼고 원래 대로 그리기
    for (size_t i = 0; i < rtvs.size(); i++) {
        context->ClearRenderTargetView(rtvs[i], clearColor);
    }
    context->OMSetRenderTargets(
        UINT(rtvs.size()), rtvs.data(),
        GraphicsManager::Instance().m_depthStencilView.Get());

    // 그림자맵들도 공용 텍스춰들 이후에 추가
    // 주의: 마지막 shadowDSV를 RenderTarget에서 해제한 후 설정
    std::vector<ID3D11ShaderResourceView *> shadowSRVs;
    for (int i = 0; i < MAX_LIGHTS; i++) {
        shadowSRVs.push_back(GraphicsManager::Instance().m_shadowSRVs[i].Get());
    }
    context->PSSetShaderResources(15, UINT(shadowSRVs.size()),
                                  shadowSRVs.data());

    context->ClearDepthStencilView(
        GraphicsManager::Instance().m_depthStencilView.Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    GraphicsManager::Instance().SetPipelineState(Graphics::defaultSolidPSO);
    GraphicsManager::Instance().SetGlobalConsts(manager_->m_globalConstsGPU);

    for (auto &i : manager_->m_basicList) {
        Renderer *renderer = nullptr;
        i->GetComponent(EnumComponentType::eRenderer,
                        (Component **)(&renderer));
        renderer->Render(context);
    }

    // 거울 반사를 그릴 필요가 없으면 불투명 거울만 그리기
    if (manager_->m_mirrorAlpha == 1.0f) {

        Renderer *renderer = nullptr;
        manager_->m_mirror->GetComponent(EnumComponentType::eRenderer,
                                         (Component **)(&renderer));
        renderer->Render(context);
    }

    GraphicsManager::Instance().SetPipelineState(Graphics::normalsPSO);
    for (auto &i : manager_->m_basicList) {

        Renderer *renderer = nullptr;
        i->GetComponent(EnumComponentType::eRenderer,
                        (Component **)(&renderer));
        if (renderer->m_drawNormals)
            renderer->RenderNormals(context);
    }

    if (true) {
        GraphicsManager::Instance().SetPipelineState(Graphics::skyboxSolidPSO);
        Renderer *renderer = nullptr;
        manager_->skybox->GetComponent(EnumComponentType::eRenderer,
                                       (Component **)(&renderer));
        renderer->Render(context);
    }

    if (manager_->m_mirrorAlpha < 1.0f) { // 거울을 그려야 하는 상황

        // 거울 2. 거울 위치만 StencilBuffer에 1로 표기
        GraphicsManager::Instance().SetPipelineState(Graphics::stencilMaskPSO);

        if (true) {
            Renderer *renderer = nullptr;
            manager_->m_mirror->GetComponent(EnumComponentType::eRenderer,
                                             (Component **)(&renderer));
            renderer->Render(context);
        }

        // 거울 3. 거울 위치에 반사된 물체들을 렌더링
        GraphicsManager::Instance().SetPipelineState(
            manager_->m_drawAsWire ? Graphics::reflectWirePSO
                                   : Graphics::reflectSolidPSO);
        GraphicsManager::Instance().SetGlobalConsts(
            manager_->m_reflectGlobalConstsGPU);

        context->ClearDepthStencilView(
            GraphicsManager::Instance().m_depthStencilView.Get(),
            D3D11_CLEAR_DEPTH, 1.0f, 0);

        for (auto &i : manager_->m_basicList) {
            Renderer *renderer = nullptr;
            i->GetComponent(EnumComponentType::eRenderer,
                            (Component **)(&renderer));
            renderer->Render(context);
        }

        if (true) {
            GraphicsManager::Instance().SetPipelineState(
                manager_->m_drawAsWire ? Graphics::reflectSkyboxWirePSO
                                       : Graphics::reflectSkyboxSolidPSO);
            Renderer *renderer = nullptr;
            manager_->skybox->GetComponent(EnumComponentType::eRenderer,
                                           (Component **)(&renderer));
            renderer->Render(context);
        }

        if (true) {
            // 거울 4. 거울 자체의 재질을 "Blend"로 그림
            GraphicsManager::Instance().SetPipelineState(
                manager_->m_drawAsWire ? Graphics::mirrorBlendWirePSO
                                       : Graphics::mirrorBlendSolidPSO);
            GraphicsManager::Instance().SetGlobalConsts(
                manager_->m_globalConstsGPU);
            Renderer *renderer = nullptr;
            manager_->m_mirror->GetComponent(EnumComponentType::eRenderer,
                                             (Component **)(&renderer));
            renderer->Render(context);
        }

    } // end of if (m_mirrorAlpha < 1.0f)

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

    if (true) {
        Renderer *renderer = nullptr;
        manager_->m_screenSquare->GetComponent(
            EnumComponentType::eRenderer, (Component **)(&renderer));
        renderer->Render(context);
    }

    GraphicsManager::Instance().SetPipelineState(Graphics::postProcessingPSO);
    manager_->m_postProcess.Render(context);
    // clang-format off
    
    imgui_->PushNode(dynamic_cast<common::INode*>(manager_.get())); 

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
        GraphicsManager::Instance().swap_chain->Present(1, 0);
    } else {
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