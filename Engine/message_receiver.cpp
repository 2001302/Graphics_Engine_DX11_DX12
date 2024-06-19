#include "message_receiver.h"

using namespace Engine;
using namespace DirectX::SimpleMath;

bool MessageReceiver::OnRightDragRequest(PipelineManager *manager,
                                         std::shared_ptr<Input> input) {
    DIMOUSESTATE mouseState;
    if (FAILED(input->Mouse()->GetDeviceState(sizeof(DIMOUSESTATE),
                                              &mouseState))) {
        // retry
        input->Mouse()->Acquire();
    } else {
        auto viewPort = Direct3D::GetInstance().viewport_;

        // mouse move vector
        Vector2 vector = Vector2(-mouseState.lX, -mouseState.lY);

        Vector3 origin(manager->camera->position.x,
                               manager->camera->position.y,
                               manager->camera->position.z);

        // convert to spherical coordinates
        double r = origin.Length();
        double phi = acos(origin.y / r);
        double theta = atan2(origin.z, origin.x);

        // rotation
        double deltaTheta = (2 * M_PI) * (vector.x / viewPort.Width);
        double deltaPhi = (2 * M_PI) * (vector.y / viewPort.Width);

        theta += deltaTheta;
        phi += deltaPhi;

        // convert to Cartesian coordinates after rotation
        double x = r * sin(phi) * cos(theta);
        double y = r * cos(phi);
        double z = r * sin(phi) * sin(theta);

        Vector3 origin_prime(x, y, z);

        if (0.0f < phi && phi < M_PI)
            manager->camera->position = Vector3(
                origin_prime.x, origin_prime.y, origin_prime.z);
    }

    return true;
}

bool MessageReceiver::OnMouseWheelRequest(PipelineManager *manager,
                                          std::shared_ptr<Input> input) {
    DIMOUSESTATE mouseState;
    if (FAILED(input->Mouse()->GetDeviceState(sizeof(DIMOUSESTATE),
                                              &mouseState))) {
        // retry
        input->Mouse()->Acquire();
    } else {
        float wheel = -mouseState.lZ / (600.0);
        Vector3 origin(manager->camera->position.x,
                               manager->camera->position.y,
                               manager->camera->position.z);

        Matrix R1(1.0f + wheel, 0.0f, 0.0f, 0.0f, 
                0.0f, 1.0f + wheel, 0.0f, 0.0f, 
                0.0f, 0.0f, 1.0f + wheel ,0.0f, 
                0.0f, 0.0f, 0.0f, 1.0f);

        auto origin_prime = Vector3::Transform(origin, R1);
        manager->camera->position = Vector3(
            origin_prime.x, origin_prime.y, origin_prime.z);
    }

    return true;
}

bool MessageReceiver::OnModelLoadRequest(PipelineManager *manager,
                                         HWND main_window) {
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
    ofn.hwndOwner = main_window;
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

        std::string fullPath = ToString(ofn.lpstrFile);
        size_t lastSlash = fullPath.find_last_of('\\');
        std::string fileName = fullPath.substr(lastSlash + 1);
        std::string directoryPath = fullPath.substr(0, lastSlash) + "\\";

        auto model = std::make_shared<GameObject>();
        manager->models[model->GetEntityId()] = model;
        GeometryGenerator::ReadFromFile(model.get(), directoryPath, fileName);
        model->SetName(fileName);

        // create constant buffer(Phong Shader)
        model->phongShader = std::make_shared<PhongShaderSource>();
        model->phongShader->vertex_constant_buffer_data.model =
            Matrix();
        model->phongShader->vertex_constant_buffer_data.view =
            Matrix();
        model->phongShader->vertex_constant_buffer_data.projection =
            Matrix();

        manager->phongShader->CreateConstantBuffer(
            model->phongShader->vertex_constant_buffer_data,
            model->phongShader->vertex_constant_buffer);
        manager->phongShader->CreateConstantBuffer(
            model->phongShader->pixel_constant_buffer_data,
            model->phongShader->pixel_constant_buffer);

        model->transform = Matrix();

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

bool MessageReceiver::OnSphereLoadRequest(PipelineManager *manager) {
    
    auto model = std::make_shared<GameObject>();
    model->SetName("sphere");
    manager->models[model->GetEntityId()] = model;

    GeometryGenerator::MakeSphere(model.get(), 1.5f, 15, 13);

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
        Matrix();
    model->phongShader->vertex_constant_buffer_data.view =
        Matrix();
    model->phongShader->vertex_constant_buffer_data.projection =
        Matrix();

    manager->phongShader->CreateConstantBuffer(
        model->phongShader->vertex_constant_buffer_data,
        model->phongShader->vertex_constant_buffer);
    manager->phongShader->CreateConstantBuffer(
        model->phongShader->pixel_constant_buffer_data,
        model->phongShader->pixel_constant_buffer);

    model->transform = Matrix();

    return true;
}

bool MessageReceiver::OnBoxLoadRequest(PipelineManager *manager) {

    auto model = std::make_shared<GameObject>();
    model->SetName("box");
    manager->models[model->GetEntityId()] = model;

    GeometryGenerator::MakeBox(model.get());

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
        Matrix();
    model->phongShader->vertex_constant_buffer_data.view =
        Matrix();
    model->phongShader->vertex_constant_buffer_data.projection =
        Matrix();

    manager->phongShader->CreateConstantBuffer(
        model->phongShader->vertex_constant_buffer_data,
        model->phongShader->vertex_constant_buffer);
    manager->phongShader->CreateConstantBuffer(
        model->phongShader->pixel_constant_buffer_data,
        model->phongShader->pixel_constant_buffer);

    model->transform = Matrix();

    return true;
}

bool MessageReceiver::OnCylinderLoadRequest(PipelineManager *manager) {

    auto model = std::make_shared<GameObject>();
    model->SetName("cylinder");
    manager->models[model->GetEntityId()] = model;

    GeometryGenerator::MakeCylinder(model.get(), 5.0f, 5.0f, 15.0f, 30);

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
        Matrix();
    model->phongShader->vertex_constant_buffer_data.view =
        Matrix();
    model->phongShader->vertex_constant_buffer_data.projection =
        Matrix();

    manager->phongShader->CreateConstantBuffer(
        model->phongShader->vertex_constant_buffer_data,
        model->phongShader->vertex_constant_buffer);
    manager->phongShader->CreateConstantBuffer(
        model->phongShader->pixel_constant_buffer_data,
        model->phongShader->pixel_constant_buffer);

    model->transform = Matrix();

    return true;
}
