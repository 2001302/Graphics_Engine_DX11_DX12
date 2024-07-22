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

    // 환경 박스 초기화
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

    // 바닥(거울)
    {
        auto mesh = GeometryGenerator::MakeSquare(5.0);
        // mesh.albedoTextureFilename =
        //     "../Assets/Textures/blender_uv_grid_2k.png";
        manager_->m_ground =
            std::make_shared<Model>(device, context, std::vector{mesh});
        manager_->m_ground->m_materialConstsCPU.albedoFactor = Vector3(0.1f);
        manager_->m_ground->m_materialConstsCPU.emissionFactor = Vector3(0.0f);
        manager_->m_ground->m_materialConstsCPU.metallicFactor = 0.5f;
        manager_->m_ground->m_materialConstsCPU.roughnessFactor = 0.3f;

        Vector3 position = Vector3(0.0f, -0.5f, 2.0f);
        manager_->m_ground->UpdateWorldRow(
            Matrix::CreateRotationX(3.141592f * 0.5f) *
            Matrix::CreateTranslation(position));

        manager_->m_mirrorPlane =
            DirectX::SimpleMath::Plane(position, Vector3(0.0f, 1.0f, 0.0f));
        manager_->m_mirror = manager_->m_ground; // 바닥에 거울처럼 반사 구현

        // m_basicList.push_back(m_ground); // 거울은 리스트에 등록 X
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

    // 추가 물체2
    {
        MeshData mesh = GeometryGenerator::MakeBox(0.2f);
        Vector3 center(0.0f, 0.5f, 2.5f);
        auto m_obj =
            std::make_shared<Model>(device, context, std::vector{mesh});
        m_obj->UpdateWorldRow(Matrix::CreateTranslation(center));
        m_obj->m_materialConstsCPU.albedoFactor = Vector3(1.0f, 0.2f, 0.2f);
        m_obj->m_materialConstsCPU.roughnessFactor = 0.5f;
        m_obj->m_materialConstsCPU.metallicFactor = 0.9f;
        m_obj->m_materialConstsCPU.emissionFactor = Vector3(0.0f);
        m_obj->UpdateConstantBuffers(device, context);

        manager_->m_basicList.push_back(m_obj);
    }

    // 조명 설정
    {
        // 조명 0은 고정
        manager_->m_globalConstsCPU.lights[0].radiance = Vector3(5.0f);
        manager_->m_globalConstsCPU.lights[0].position =
            Vector3(0.0f, 1.5f, 1.1f);
        manager_->m_globalConstsCPU.lights[0].direction =
            Vector3(0.0f, -1.0f, 0.0f);
        manager_->m_globalConstsCPU.lights[0].spotPower = 3.0f;
        manager_->m_globalConstsCPU.lights[0].radius = 0.02f;
        manager_->m_globalConstsCPU.lights[0].type =
            LIGHT_SPOT | LIGHT_SHADOW; // Point with shadow

        // 조명 1의 위치와 방향은 Update()에서 설정
        manager_->m_globalConstsCPU.lights[1].radiance = Vector3(5.0f);
        manager_->m_globalConstsCPU.lights[1].spotPower = 3.0f;
        manager_->m_globalConstsCPU.lights[1].fallOffEnd = 20.0f;
        manager_->m_globalConstsCPU.lights[1].radius = 0.02f;
        manager_->m_globalConstsCPU.lights[1].type =
            LIGHT_SPOT | LIGHT_SHADOW; // Point with shadow

        // 조명 2는 꺼놓음
        manager_->m_globalConstsCPU.lights[2].type = LIGHT_OFF;
    }

    // 조명 위치 표시
    {
        for (int i = 0; i < MAX_LIGHTS; i++) {
            MeshData sphere = GeometryGenerator::MakeSphere(1.0f, 20, 20);
            manager_->m_lightSphere[i] =
                std::make_shared<Model>(device, context, std::vector{sphere});
            manager_->m_lightSphere[i]->UpdateWorldRow(
                Matrix::CreateTranslation(
                    manager_->m_globalConstsCPU.lights[i].position));
            manager_->m_lightSphere[i]->m_materialConstsCPU.albedoFactor =
                Vector3(0.0f);
            manager_->m_lightSphere[i]->m_materialConstsCPU.emissionFactor =
                Vector3(1.0f, 1.0f, 0.0f);
            manager_->m_lightSphere[i]->m_castShadow =
                false; // 조명 표시 물체들은 그림자 X

            if (manager_->m_globalConstsCPU.lights[i].type == 0)
                manager_->m_lightSphere[i]->m_isVisible = false;

            manager_->m_basicList.push_back(
                manager_->m_lightSphere[i]); // 리스트에 등록
        }
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

    // std::map<EnumDataBlockType, common::IDataBlock *> dataBlock = {
    //     {EnumDataBlockType::eManager, manager_.get()},
    //     {EnumDataBlockType::eGui, imgui_.get()},
    // };

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

    // 반사 행렬 추가
    const Vector3 eyeWorld = manager_->camera->GetEyePos();
    const Matrix reflectRow = Matrix::CreateReflection(manager_->m_mirrorPlane);
    const Matrix viewRow = manager_->camera->GetViewRow();
    const Matrix projRow = manager_->camera->GetProjRow();

    UpdateLights(dt);

    // 공용 ConstantBuffer 업데이트
    UpdateGlobalConstants(eyeWorld, viewRow, projRow, reflectRow);

    // 거울은 따로 처리
    manager_->m_mirror->UpdateConstantBuffers(device, context);

    // 조명의 위치 반영
    for (int i = 0; i < MAX_LIGHTS; i++)
        manager_->m_lightSphere[i]->UpdateWorldRow(
            Matrix::CreateScale((std::max)(
                0.01f, manager_->m_globalConstsCPU.lights[i].radius)) *
            Matrix::CreateTranslation(
                manager_->m_globalConstsCPU.lights[i].position));

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
        i->UpdateConstantBuffers(device, context);
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

    for (auto &i : manager_->m_basicList)
        i->Render(context);
    manager_->skybox->Render(context);
    manager_->m_mirror->Render(context);

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
            for (auto &i : manager_->m_basicList)
                if (i->m_castShadow && i->m_isVisible)
                    i->Render(context);
            manager_->skybox->Render(context);
            manager_->m_mirror->Render(context);
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
        i->Render(context);
    }

    // 거울 반사를 그릴 필요가 없으면 불투명 거울만 그리기
    if (manager_->m_mirrorAlpha == 1.0f)
        manager_->m_mirror->Render(context);

    GraphicsManager::Instance().SetPipelineState(Graphics::normalsPSO);
    for (auto &i : manager_->m_basicList) {
        if (i->m_drawNormals)
            i->RenderNormals(context);
    }

    GraphicsManager::Instance().SetPipelineState(Graphics::skyboxSolidPSO);
    manager_->skybox->Render(context);

    if (manager_->m_mirrorAlpha < 1.0f) { // 거울을 그려야 하는 상황

        // 거울 2. 거울 위치만 StencilBuffer에 1로 표기
        GraphicsManager::Instance().SetPipelineState(Graphics::stencilMaskPSO);

        manager_->m_mirror->Render(context);

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
            i->Render(context);
        }

        GraphicsManager::Instance().SetPipelineState(
            manager_->m_drawAsWire ? Graphics::reflectSkyboxWirePSO
                                   : Graphics::reflectSkyboxSolidPSO);
        manager_->skybox->Render(context);

        // 거울 4. 거울 자체의 재질을 "Blend"로 그림
        GraphicsManager::Instance().SetPipelineState(
            manager_->m_drawAsWire ? Graphics::mirrorBlendWirePSO
                                   : Graphics::mirrorBlendSolidPSO);
        GraphicsManager::Instance().SetGlobalConsts(
            manager_->m_globalConstsGPU);

        manager_->m_mirror->Render(context);

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
    manager_->m_screenSquare->Render(context);

    GraphicsManager::Instance().SetPipelineState(Graphics::postProcessingPSO);
    manager_->m_postProcess.Render(context);
    // clang-format off
    
    imgui_->PushNode(dynamic_cast<common::INodeUi*>(manager_.get())); 

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