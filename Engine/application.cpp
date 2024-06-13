#include "Application.h"

using namespace Engine;

Application::~Application() {

    return;
}

bool Application::OnStart() {
    
    Platform::OnStart();
    
    m_manager = new PipelineManager();
    m_env = new Env();
    m_imgui = new ImGuiManager();

    m_env->screenWidth = m_screenWidth;
    m_env->screenHeight = m_screenHeight;

    // Create and initialize the input object.  This object will be used to
    // handle reading the keyboard input from the user.
    m_input = std::make_unique<Input>();

    m_input->Initialize(m_hinstance, m_mainWindow, m_screenWidth, m_screenHeight);

    std::map<EnumDataBlockType, IDataBlock *> dataBlock = {
        {EnumDataBlockType::eManager, m_manager},
        {EnumDataBlockType::eGui, m_imgui},
    };

    Direct3D::GetInstance().Init(m_env, VSYNC_ENABLED, m_mainWindow,
                                 FULL_SCREEN);

    auto tree = new BehaviorTreeBuilder();

    tree->Build(dataBlock)
        ->Sequence()
        ->Excute(std::make_shared<InitializeCamera>())
        ->Excute(std::make_shared<InitializePhongShader>(m_mainWindow))
        ->Close();

    tree->Run();

    m_imgui->Initialize(m_mainWindow, &Direct3D::GetInstance());

    return true;
}

bool Application::OnFrame() {

    std::map<EnumDataBlockType, IDataBlock*> dataBlock = {
    {EnumDataBlockType::eManager, m_manager},
    {EnumDataBlockType::eEnv, m_env},
    {EnumDataBlockType::eGui, m_imgui},
    };

    // Clear the buffers to begin the scene.
    Direct3D::GetInstance().BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    if (!m_input->Frame()) {
        return false;
    }
    // ImGui
    m_imgui->Prepare(m_env);

    auto tree = std::make_unique<BehaviorTreeBuilder>();

    tree->Build(dataBlock)
        ->Sequence()
        ->Excute(std::make_shared<RenderGameObjects>())
        ->Close();

    tree->Run();

    m_imgui->Render(m_mainWindow);

    // Present the rendered scene to the screen.
    Direct3D::GetInstance().EndScene();

    return true;
}

bool Application::OnStop() 
{
    Platform::OnStop();

    if (m_manager) {
        for (auto& model : m_manager->models) {
            delete model;
            model = 0;
        }

        m_manager->phongShader.reset();
        m_manager->camera.reset();
    }

    if (m_env) {
        delete m_env;
        m_env = 0;
    }

    if (m_imgui) {
        m_imgui->Shutdown();
        delete m_imgui;
        m_imgui = 0;
    }

    if (m_input) {
        m_input->Shutdown();
        m_input.reset();
    }

    return true;
}
bool Application::OnRightDragRequest() {
    DIMOUSESTATE mouseState;
    if (FAILED(m_input->Mouse()->GetDeviceState(sizeof(DIMOUSESTATE),
                                                &mouseState))) {
        // retry
        m_input->Mouse()->Acquire();
    } else {
        auto viewPort = Direct3D::GetInstance().viewport;

        // mouse move vector
        Eigen::Vector2d vector =
            Eigen::Vector2d(-mouseState.lX, -mouseState.lY);

        Eigen::Vector3d origin(m_manager->camera->position.x,
                               m_manager->camera->position.y,
                               m_manager->camera->position.z);

        // convert to spherical coordinates
        double r = origin.norm();
        double phi = acos(origin.y() / r);
        double theta = atan2(origin.z(), origin.x());

        // rotation
        double deltaTheta = (2 * M_PI) * (vector.x() / viewPort.Width);
        double deltaPhi = (2 * M_PI) * (vector.y() / viewPort.Width);

        theta += deltaTheta;
        phi += deltaPhi;

        // convert to Cartesian coordinates after rotation
        double x = r * sin(phi) * cos(theta);
        double y = r * cos(phi);
        double z = r * sin(phi) * sin(theta);

        Eigen::Vector3d origin_prime(x, y, z);

        if (0.0f < phi && phi < M_PI)
            m_manager->camera->position = DirectX::SimpleMath::Vector3(
                origin_prime.x(), origin_prime.y(), origin_prime.z());
    }

    return true;
}

bool Application::OnMouseWheelRequest() {
    DIMOUSESTATE mouseState;
    if (FAILED(m_input->Mouse()->GetDeviceState(sizeof(DIMOUSESTATE),
                                                &mouseState))) {
        // retry
        m_input->Mouse()->Acquire();
    } else {
        double wheel = -mouseState.lZ / (600.0);
        Eigen::Vector3d origin(m_manager->camera->position.x,
                               m_manager->camera->position.y,
                               m_manager->camera->position.z);

        Eigen::Matrix3d R1;
        R1 << 1.0 + wheel, 0.0, 0.0, 0.0, 1.0 + wheel, 0.0, 0.0, 0.0,
            1.0 + wheel;

        Eigen::Vector3d origin_prime = R1 * origin;
        m_manager->camera->position = DirectX::SimpleMath::Vector3(
            origin_prime.x(), origin_prime.y(), origin_prime.z());
    }

    return true;
}

bool Application::OnModelLoadRequest() {
    auto ToString = [](LPWSTR lpwstr) -> std::string {
        if (!lpwstr)
            return std::string();

        int len =
            WideCharToMultiByte(CP_UTF8, 0, lpwstr, -1, NULL, 0, NULL, NULL);
        std::string result(len, '\0');
        WideCharToMultiByte(CP_UTF8, 0, lpwstr, -1, &result[0], len, NULL,
                            NULL);
        return result;
    };

    OPENFILENAMEW ofn;
    wchar_t szFile[260] = {0};

    // OPENFILENAME struct initialize
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_mainWindow;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"All files\0*.*\0Text Files\0*.TXT\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // show file explorer
    if (GetOpenFileName(&ofn)) {
        m_manager->models.push_back(new GameObject());
        auto model = m_manager->models.back();

        std::string fullPath = ToString(ofn.lpstrFile);
        size_t lastSlash = fullPath.find_last_of('\\');
        std::string fileName = fullPath.substr(lastSlash + 1);
        std::string directoryPath = fullPath.substr(0, lastSlash) + "\\";

        model = GeometryGenerator::ReadFromFile(model, directoryPath, fileName);

        // create constant buffer(Phong Shader)
        model->phongShader = std::make_shared<PhongShaderSource>();
        model->phongShader->vertex_constant_buffer_data.model =
            DirectX::SimpleMath::Matrix();
        model->phongShader->vertex_constant_buffer_data.view =
            DirectX::SimpleMath::Matrix();
        model->phongShader->vertex_constant_buffer_data.projection =
            DirectX::SimpleMath::Matrix();

        m_manager->phongShader->CreateConstantBuffer(
            model->phongShader->vertex_constant_buffer_data,
            model->phongShader->vertex_constant_buffer);
        m_manager->phongShader->CreateConstantBuffer(
            model->phongShader->pixel_constant_buffer_data,
            model->phongShader->pixel_constant_buffer);

        model->transform = DirectX::SimpleMath::Matrix();

        for (const auto &meshData : model->meshes) {
            {
                D3D11_BUFFER_DESC bufferDesc;
                ZeroMemory(&bufferDesc, sizeof(bufferDesc));
                bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
                bufferDesc.ByteWidth =
                    sizeof(Engine::Vertex) *
                    meshData->vertices
                        .size(); // UINT(sizeof(T_VERTEX) * vertices.size());
                bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                bufferDesc.CPUAccessFlags =
                    0; // 0 if no CPU access is necessary.
                bufferDesc.StructureByteStride = sizeof(Engine::Vertex);
                bufferDesc.MiscFlags = 0;

                D3D11_SUBRESOURCE_DATA vertexBufferData = {
                    0}; // MS 예제에서 초기화하는 방식
                vertexBufferData.pSysMem = meshData->vertices.data();
                vertexBufferData.SysMemPitch = 0;
                vertexBufferData.SysMemSlicePitch = 0;

                const HRESULT hr =
                    Direct3D::GetInstance().GetDevice()->CreateBuffer(
                        &bufferDesc, &vertexBufferData,
                        &meshData->vertexBuffer);
                if (FAILED(hr)) {
                    std::cout << "CreateBuffer() failed. " << std::hex << hr
                              << std::endl;
                };
            }
            {
                D3D11_BUFFER_DESC bufferDesc;
                bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
                bufferDesc.ByteWidth =
                    sizeof(unsigned long) * meshData->indices.size();
                bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
                bufferDesc.CPUAccessFlags =
                    0; // 0 if no CPU access is necessary.
                bufferDesc.StructureByteStride = sizeof(int);
                bufferDesc.MiscFlags = 0;

                D3D11_SUBRESOURCE_DATA indexBufferData = {0};
                indexBufferData.pSysMem = meshData->indices.data();
                indexBufferData.SysMemPitch = 0;
                indexBufferData.SysMemSlicePitch = 0;

                Direct3D::GetInstance().GetDevice()->CreateBuffer(
                    &bufferDesc, &indexBufferData, &meshData->indexBuffer);
            }

            if (!meshData->textureFilename.empty()) {

                std::cout << meshData->textureFilename << std::endl;

                ResourceHelper::CreateTexture(meshData->textureFilename,
                                              meshData->texture,
                                              meshData->textureResourceView);
            }
        }

    } else {
        // need logger
    }
    return true;
}

bool Application::OnSphereLoadRequest() {
    m_manager->models.push_back(new GameObject());

    auto model = m_manager->models.back();

    GeometryGenerator::MakeSphere(model, 1.5f, 15, 13);

    for (auto mesh : model->meshes) {
        ResourceHelper::CreateTexture(
            "C:\\Users\\user\\Source\\repos\\Engine\\Engine\\data\\ojwD8.jpg",
            mesh->texture, mesh->textureResourceView);

        {
            D3D11_BUFFER_DESC bufferDesc;
            ZeroMemory(&bufferDesc, sizeof(bufferDesc));
            bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
            bufferDesc.ByteWidth =
                sizeof(Engine::Vertex) *
                mesh->vertices
                    .size(); // UINT(sizeof(T_VERTEX) * vertices.size());
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bufferDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
            bufferDesc.StructureByteStride = sizeof(Engine::Vertex);
            bufferDesc.MiscFlags = 0;

            D3D11_SUBRESOURCE_DATA vertexBufferData = {
                0}; // MS 예제에서 초기화하는 방식
            vertexBufferData.pSysMem = mesh->vertices.data();
            vertexBufferData.SysMemPitch = 0;
            vertexBufferData.SysMemSlicePitch = 0;

            const HRESULT hr =
                Direct3D::GetInstance().GetDevice()->CreateBuffer(
                    &bufferDesc, &vertexBufferData, &mesh->vertexBuffer);
            if (FAILED(hr)) {
                std::cout << "CreateBuffer() failed. " << std::hex << hr
                          << std::endl;
            };
        }
        {
            D3D11_BUFFER_DESC bufferDesc;
            bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
            bufferDesc.ByteWidth = sizeof(unsigned long) * mesh->indices.size();
            bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            bufferDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
            bufferDesc.StructureByteStride = sizeof(int);
            bufferDesc.MiscFlags = 0;

            D3D11_SUBRESOURCE_DATA indexBufferData = {0};
            indexBufferData.pSysMem = mesh->indices.data();
            indexBufferData.SysMemPitch = 0;
            indexBufferData.SysMemSlicePitch = 0;

            Direct3D::GetInstance().GetDevice()->CreateBuffer(
                &bufferDesc, &indexBufferData, &mesh->indexBuffer);
        }
    }

    // create constant buffer(Phong Shader)
    model->phongShader = std::make_shared<PhongShaderSource>();
    model->phongShader->vertex_constant_buffer_data.model =
        DirectX::SimpleMath::Matrix();
    model->phongShader->vertex_constant_buffer_data.view =
        DirectX::SimpleMath::Matrix();
    model->phongShader->vertex_constant_buffer_data.projection =
        DirectX::SimpleMath::Matrix();

    m_manager->phongShader->CreateConstantBuffer(
        model->phongShader->vertex_constant_buffer_data,
        model->phongShader->vertex_constant_buffer);
    m_manager->phongShader->CreateConstantBuffer(
        model->phongShader->pixel_constant_buffer_data,
        model->phongShader->pixel_constant_buffer);

    model->transform = DirectX::SimpleMath::Matrix();

    return true;
}

bool Application::OnBoxLoadRequest() {
    m_manager->models.push_back(new GameObject());

    auto model = m_manager->models.back();

    GeometryGenerator::MakeBox(model);

    for (auto mesh : model->meshes) {
        ResourceHelper::CreateTexture(
            "C:\\Users\\user\\Source\\repos\\Engine\\Engine\\data\\crate2_"
            "diffuse.png",
            mesh->texture, mesh->textureResourceView);

        {
            D3D11_BUFFER_DESC bufferDesc;
            ZeroMemory(&bufferDesc, sizeof(bufferDesc));
            bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
            bufferDesc.ByteWidth =
                sizeof(Engine::Vertex) *
                mesh->vertices
                    .size(); // UINT(sizeof(T_VERTEX) * vertices.size());
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bufferDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
            bufferDesc.StructureByteStride = sizeof(Engine::Vertex);
            bufferDesc.MiscFlags = 0;

            D3D11_SUBRESOURCE_DATA vertexBufferData = {
                0}; // MS 예제에서 초기화하는 방식
            vertexBufferData.pSysMem = mesh->vertices.data();
            vertexBufferData.SysMemPitch = 0;
            vertexBufferData.SysMemSlicePitch = 0;

            const HRESULT hr =
                Direct3D::GetInstance().GetDevice()->CreateBuffer(
                    &bufferDesc, &vertexBufferData, &mesh->vertexBuffer);
            if (FAILED(hr)) {
                std::cout << "CreateBuffer() failed. " << std::hex << hr
                          << std::endl;
            };
        }
        {
            D3D11_BUFFER_DESC bufferDesc;
            bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
            bufferDesc.ByteWidth = sizeof(unsigned long) * mesh->indices.size();
            bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            bufferDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
            bufferDesc.StructureByteStride = sizeof(int);
            bufferDesc.MiscFlags = 0;

            D3D11_SUBRESOURCE_DATA indexBufferData = {0};
            indexBufferData.pSysMem = mesh->indices.data();
            indexBufferData.SysMemPitch = 0;
            indexBufferData.SysMemSlicePitch = 0;

            Direct3D::GetInstance().GetDevice()->CreateBuffer(
                &bufferDesc, &indexBufferData, &mesh->indexBuffer);
        }
    }

    // create constant buffer(Phong Shader)
    model->phongShader = std::make_shared<PhongShaderSource>();
    model->phongShader->vertex_constant_buffer_data.model =
        DirectX::SimpleMath::Matrix();
    model->phongShader->vertex_constant_buffer_data.view =
        DirectX::SimpleMath::Matrix();
    model->phongShader->vertex_constant_buffer_data.projection =
        DirectX::SimpleMath::Matrix();

    m_manager->phongShader->CreateConstantBuffer(
        model->phongShader->vertex_constant_buffer_data,
        model->phongShader->vertex_constant_buffer);
    m_manager->phongShader->CreateConstantBuffer(
        model->phongShader->pixel_constant_buffer_data,
        model->phongShader->pixel_constant_buffer);

    model->transform = DirectX::SimpleMath::Matrix();

    return true;
}

bool Application::OnCylinderLoadRequest() {
    m_manager->models.push_back(new GameObject());

    auto model = m_manager->models.back();

    GeometryGenerator::MakeCylinder(model, 5.0f, 5.0f, 15.0f, 30);

    for (auto mesh : model->meshes) {
        ResourceHelper::CreateTexture(
            "C:\\Users\\user\\Source\\repos\\Engine\\Engine\\data\\wall.jpg",
            mesh->texture, mesh->textureResourceView);

        {
            D3D11_BUFFER_DESC bufferDesc;
            ZeroMemory(&bufferDesc, sizeof(bufferDesc));
            bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
            bufferDesc.ByteWidth =
                sizeof(Engine::Vertex) *
                mesh->vertices
                    .size(); // UINT(sizeof(T_VERTEX) * vertices.size());
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bufferDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
            bufferDesc.StructureByteStride = sizeof(Engine::Vertex);
            bufferDesc.MiscFlags = 0;

            D3D11_SUBRESOURCE_DATA vertexBufferData = {
                0}; // MS 예제에서 초기화하는 방식
            vertexBufferData.pSysMem = mesh->vertices.data();
            vertexBufferData.SysMemPitch = 0;
            vertexBufferData.SysMemSlicePitch = 0;

            const HRESULT hr =
                Direct3D::GetInstance().GetDevice()->CreateBuffer(
                    &bufferDesc, &vertexBufferData, &mesh->vertexBuffer);
            if (FAILED(hr)) {
                std::cout << "CreateBuffer() failed. " << std::hex << hr
                          << std::endl;
            };
        }
        {
            D3D11_BUFFER_DESC bufferDesc;
            bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
            bufferDesc.ByteWidth = sizeof(unsigned long) * mesh->indices.size();
            bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            bufferDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
            bufferDesc.StructureByteStride = sizeof(int);
            bufferDesc.MiscFlags = 0;

            D3D11_SUBRESOURCE_DATA indexBufferData = {0};
            indexBufferData.pSysMem = mesh->indices.data();
            indexBufferData.SysMemPitch = 0;
            indexBufferData.SysMemSlicePitch = 0;

            Direct3D::GetInstance().GetDevice()->CreateBuffer(
                &bufferDesc, &indexBufferData, &mesh->indexBuffer);
        }
    }

    // create constant buffer(Phong Shader)
    model->phongShader = std::make_shared<PhongShaderSource>();
    model->phongShader->vertex_constant_buffer_data.model =
        DirectX::SimpleMath::Matrix();
    model->phongShader->vertex_constant_buffer_data.view =
        DirectX::SimpleMath::Matrix();
    model->phongShader->vertex_constant_buffer_data.projection =
        DirectX::SimpleMath::Matrix();

    m_manager->phongShader->CreateConstantBuffer(
        model->phongShader->vertex_constant_buffer_data,
        model->phongShader->vertex_constant_buffer);
    m_manager->phongShader->CreateConstantBuffer(
        model->phongShader->pixel_constant_buffer_data,
        model->phongShader->pixel_constant_buffer);

    model->transform = DirectX::SimpleMath::Matrix();

    return true;
}

LRESULT CALLBACK Application::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
    extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    ImGui_ImplWin32_WndProcHandler(hwnd, umsg, wparam, lparam);

    switch (umsg)
    {
    case WM_MOUSEMOVE:
    {
        if (wparam & MK_RBUTTON)
        {
            return OnRightDragRequest();
        }
        break;
    }
    case WM_MOUSEWHEEL:
    {
        return OnMouseWheelRequest();
        break;
    }
    case WM_RBUTTONDOWN:
    {
        //return OnRightClickRequest();
        break;
    }
    case WM_LBUTTONUP:
    {
        break;
    }
    case WM_MODEL_LOAD:
    {
        return OnModelLoadRequest();
        break;
    }
    case WM_SPHERE_LOAD:
    {
        return OnSphereLoadRequest();
        break;
    }
    case WM_BOX_LOAD:
    {
        return OnBoxLoadRequest();
        break;
    }
    case WM_CYLINDER_LOAD:
    {
        return OnCylinderLoadRequest();
        break;
    }
    default:
    {
        return DefWindowProc(hwnd, umsg, wparam, lparam);
    }
    }
}
