#include "application.h"
#include "behavior_tree_builder.h"
#include "graphics_common.h"

namespace platform {

using namespace dx11;
using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

void SetPipelineState(const GraphicsPSO &pso) {

    auto context = dx11::GraphicsContext::Instance().device_context();
    context->VSSetShader(pso.m_vertexShader.Get(), 0, 0);
    context->PSSetShader(pso.m_pixelShader.Get(), 0, 0);
    context->HSSetShader(pso.m_hullShader.Get(), 0, 0);
    context->DSSetShader(pso.m_domainShader.Get(), 0, 0);
    context->GSSetShader(pso.m_geometryShader.Get(), 0, 0);
    context->IASetInputLayout(pso.m_inputLayout.Get());
    context->RSSetState(pso.m_rasterizerState.Get());
    context->OMSetBlendState(pso.m_blendState.Get(), pso.m_blendFactor,
                             0xffffffff);
    context->OMSetDepthStencilState(pso.m_depthStencilState.Get(),
                                    pso.m_stencilRef);
    context->IASetPrimitiveTopology(pso.m_primitiveTopology);
}

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
    //auto tree = new dx11::BehaviorTreeBuilder();
    //tree->Build(dataBlock)
    //    ->Sequence()
    //        ->Excute(std::make_shared<dx11::InitializeBoardMap>())
    //        ->Excute(std::make_shared<dx11::InitializeCamera>())
    //        ->Excute(std::make_shared<dx11::InitializeCubeMapShader>())
    //        ->Excute(std::make_shared<dx11::InitializePhongShader>())
    //        ->Excute(std::make_shared<dx11::InitializePhysicallyBasedShader>())
    //    ->Close()
    //->Run();
    // clang-format on

    InitCubemaps(L"../Engine/Assets/Textures/Cubemaps/HDRI/", L"SampleEnvHDR.dds",
                 L"SampleSpecularHDR.dds", L"SampleDiffuseHDR.dds",
                 L"SampleBrdf.dds");

    auto device = dx11::GraphicsContext::Instance().device();
    auto context = dx11::GraphicsContext::Instance().device_context();

    // 공통으로 쓰이는 ConstBuffers
    D3D11Utils::CreateConstBuffer(device, m_globalConstsCPU,
                                  m_globalConstsGPU);
    D3D11Utils::CreateConstBuffer(device, m_reflectGlobalConstsCPU,
                                  m_reflectGlobalConstsGPU);
    // 후처리용 화면 사각형
    {
        dx11::MeshData meshData = dx11::GeometryGenerator::MakeSquare();
        m_screenSquare = std::make_shared<dx11::Model>(device, context,
                                                       std::vector{meshData});
    }
    // 환경 박스 초기화
    {
        dx11::MeshData skyboxMesh = dx11::GeometryGenerator::MakeBox(40.0f);
        std::reverse(skyboxMesh.indices.begin(), skyboxMesh.indices.end());
        m_skybox =
            make_shared<dx11::Model>(device, context, std::vector{skyboxMesh});
    }

    // 바닥(거울)
    {
        auto mesh = dx11::GeometryGenerator::MakeSquare(5.0);
        // mesh.albedoTextureFilename =
        //     "../Assets/Textures/blender_uv_grid_2k.png";
        m_ground = make_shared<dx11::Model>(device, context, std::vector{mesh});
        m_ground->m_materialConstsCPU.albedoFactor = Vector3(0.1f);
        m_ground->m_materialConstsCPU.emissionFactor = Vector3(0.0f);
        m_ground->m_materialConstsCPU.metallicFactor = 0.5f;
        m_ground->m_materialConstsCPU.roughnessFactor = 0.3f;

        Vector3 position = Vector3(0.0f, -0.5f, 2.0f);
        m_ground->UpdateWorldRow(Matrix::CreateRotationX(3.141592f * 0.5f) *
                                 Matrix::CreateTranslation(position));

        m_mirrorPlane = SimpleMath::Plane(position, Vector3(0.0f, 1.0f, 0.0f));
        m_mirror = m_ground; // 바닥에 거울처럼 반사 구현

        // m_basicList.push_back(m_ground); // 거울은 리스트에 등록 X
    }

    // Main Object
    {
        vector<MeshData> meshes = {GeometryGenerator::MakeSphere(0.4f, 50, 50)};

        Vector3 center(0.0f, 0.0f, 2.0f);
        m_mainObj = make_shared<Model>(device, context, meshes);
        m_mainObj->m_materialConstsCPU.invertNormalMapY = true; // GLTF는 true로
        m_mainObj->m_materialConstsCPU.albedoFactor = Vector3(1.0f);
        m_mainObj->m_materialConstsCPU.roughnessFactor = 0.3f;
        m_mainObj->m_materialConstsCPU.metallicFactor = 0.8f;
        m_mainObj->UpdateWorldRow(Matrix::CreateTranslation(center));

        m_basicList.push_back(m_mainObj); // 리스트에 등록

        // 동일한 크기와 위치에 BoundingSphere 만들기
        m_mainBoundingSphere = BoundingSphere(center, 0.4f);
    }

    // 조명 설정
    {
        // 조명 0은 고정
        m_globalConstsCPU.lights[0].radiance = Vector3(5.0f);
        m_globalConstsCPU.lights[0].position = Vector3(0.0f, 1.5f, 1.1f);
        m_globalConstsCPU.lights[0].direction = Vector3(0.0f, -1.0f, 0.0f);
        m_globalConstsCPU.lights[0].spotPower = 3.0f;
        m_globalConstsCPU.lights[0].radius = 0.02f;
        m_globalConstsCPU.lights[0].type =
            LIGHT_SPOT | LIGHT_SHADOW; // Point with shadow

        // 조명 1의 위치와 방향은 Update()에서 설정
        m_globalConstsCPU.lights[1].radiance = Vector3(5.0f);
        m_globalConstsCPU.lights[1].spotPower = 3.0f;
        m_globalConstsCPU.lights[1].fallOffEnd = 20.0f;
        m_globalConstsCPU.lights[1].radius = 0.02f;
        m_globalConstsCPU.lights[1].type =
            LIGHT_SPOT | LIGHT_SHADOW; // Point with shadow

        // 조명 2는 꺼놓음
        m_globalConstsCPU.lights[2].type = LIGHT_OFF;
    }

    // 조명 위치 표시
    {
        for (int i = 0; i < MAX_LIGHTS; i++) {
            MeshData sphere = GeometryGenerator::MakeSphere(1.0f, 20, 20);
            m_lightSphere[i] =
                make_shared<Model>(device, context, vector{sphere});
            m_lightSphere[i]->UpdateWorldRow(Matrix::CreateTranslation(
                m_globalConstsCPU.lights[i].position));
            m_lightSphere[i]->m_materialConstsCPU.albedoFactor = Vector3(0.0f);
            m_lightSphere[i]->m_materialConstsCPU.emissionFactor =
                Vector3(1.0f, 1.0f, 0.0f);
            m_lightSphere[i]->m_castShadow =
                false; // 조명 표시 물체들은 그림자 X

            if (m_globalConstsCPU.lights[i].type == 0)
                m_lightSphere[i]->m_isVisible = false;

            m_basicList.push_back(m_lightSphere[i]); // 리스트에 등록
        }
    }

    m_postProcess.Initialize(device, context,
                             {GraphicsContext::Instance().post_effects_SRV},
                             {GraphicsContext::Instance().back_buffer_RTV()},
                             common::Env::Instance().screen_width,
                             common::Env::Instance().screen_height, 4);
    OnFrame();

    return true;
}

bool Application::OnFrame() {

    auto device = dx11::GraphicsContext::Instance().device();
    auto context = dx11::GraphicsContext::Instance().device_context();

    std::map<dx11::EnumDataBlockType, common::IDataBlock *> dataBlock = {
        {dx11::EnumDataBlockType::eManager, manager_.get()},
        {dx11::EnumDataBlockType::eGui, imgui_.get()},
    };

    // Clear the buffers to begin the scene.
    dx11::GraphicsContext::Instance().BeginScene(
        0.0f, 0.0f, 0.0f, 1.0f, imgui_->Tab().common.draw_as_wire_);

    // clang-format off

    if (imgui_->SelectedId()) {
        imgui_->PushNode(manager_->models[imgui_->SelectedId()]); 
    }

    //auto tree = std::make_unique<dx11::BehaviorTreeBuilder>();
    //tree->Build(dataBlock)
    //->Excute(std::make_shared<dx11::UpdateCamera>())
    //->Parallel(manager_->models)
    //    ->Selector()
    //        ->Sequence()
    //            ->Excute(std::make_shared<dx11::CheckPhongShader>())
    //            ->Excute(std::make_shared<dx11::UpdateGameObjectsUsingPhongShader>())
    //            ->Excute(std::make_shared<dx11::RenderGameObjectsUsingPhongShader>())
    //        ->Close()
    //        ->Sequence()
    //            ->Excute(std::make_shared<dx11::CheckPhysicallyBasedShader>())
    //            ->Excute(std::make_shared<dx11::UpdateGameObjectsUsingPhysicallyBasedShader>())
    //            ->Excute(std::make_shared<dx11::RenderGameObjectsUsingPhysicallyBasedShader>())
    //        ->Close()
    //    ->Close()
    //->Close()
    //->Conditional(std::make_shared<dx11::CheckCubeMapShader>())
    //    ->Sequence()
    //        ->Excute(std::make_shared<dx11::UpdateCubeMap>())
    //        ->Excute(std::make_shared<dx11::RenderCubeMap>())
    //    ->Close()
    //->End()
    //->Excute(std::make_shared<dx11::RenderBoardMap>())
    //->Run();
    // clang-format on

    Vector3 eyeWorld = Vector3();
    Matrix viewRow = Matrix();
    Matrix projRow = Matrix();
    Matrix reflectRow = Matrix();
    UpdateGlobalConstants(eyeWorld, viewRow, projRow, reflectRow);

    GraphicsContext::Instance().SetViewPort(
        0, 0, common::Env::Instance().screen_width,
        common::Env::Instance().screen_height);

    context->VSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                           Graphics::sampleStates.data());
    context->PSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                           Graphics::sampleStates.data());

    // 단순 이미지 처리와 블룸
    SetPipelineState(Graphics::postProcessingPSO);
    m_postProcess.Render(context);

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

void Application::InitCubemaps(std::wstring basePath, std::wstring envFilename,
                               std::wstring specularFilename,
                               std::wstring irradianceFilename,
                               std::wstring brdfFilename) {
    auto device = dx11::GraphicsContext::Instance().device();
    // BRDF LookUp Table은 CubeMap이 아니라 2D 텍스춰 입니다.
    dx11::D3D11Utils::CreateDDSTexture(device, (basePath + envFilename).c_str(),
                                       true, m_envSRV);
    dx11::D3D11Utils::CreateDDSTexture(
        device, (basePath + specularFilename).c_str(), true, m_specularSRV);
    dx11::D3D11Utils::CreateDDSTexture(
        device, (basePath + irradianceFilename).c_str(), true, m_irradianceSRV);
    dx11::D3D11Utils::CreateDDSTexture(
        device, (basePath + brdfFilename).c_str(), false, m_brdfSRV);
}

void Application::UpdateGlobalConstants(const Vector3 &eyeWorld,
                                        const Matrix &viewRow,
                                        const Matrix &projRow,
                                        const Matrix &refl = Matrix()) {

    auto device = dx11::GraphicsContext::Instance().device();
    auto context = dx11::GraphicsContext::Instance().device_context();

    m_globalConstsCPU.eyeWorld = eyeWorld;
    m_globalConstsCPU.view = viewRow.Transpose();
    m_globalConstsCPU.proj = projRow.Transpose();
    m_globalConstsCPU.invProj = projRow.Invert().Transpose();
    m_globalConstsCPU.viewProj = (viewRow * projRow).Transpose();
    // 그림자 렌더링에 사용
    m_globalConstsCPU.invViewProj = m_globalConstsCPU.viewProj.Invert();

    m_reflectGlobalConstsCPU = m_globalConstsCPU;
    memcpy(&m_reflectGlobalConstsCPU, &m_globalConstsCPU,
           sizeof(m_globalConstsCPU));
    m_reflectGlobalConstsCPU.view = (refl * viewRow).Transpose();
    m_reflectGlobalConstsCPU.viewProj = (refl * viewRow * projRow).Transpose();
    // 그림자 렌더링에 사용 (TODO: 광원의 위치도 반사시킨 후에 계산해야 함)
    m_reflectGlobalConstsCPU.invViewProj =
        m_reflectGlobalConstsCPU.viewProj.Invert();

    D3D11Utils::UpdateBuffer(device, context, m_globalConstsCPU,
                             m_globalConstsGPU);
    D3D11Utils::UpdateBuffer(device, context, m_reflectGlobalConstsCPU,
                             m_reflectGlobalConstsGPU);
}

} // namespace platform