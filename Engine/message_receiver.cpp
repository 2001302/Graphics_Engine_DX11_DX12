#include "message_receiver.h"

using namespace dx11;
using namespace DirectX::SimpleMath;

bool MessageReceiver::OnRightDragRequest(PipelineManager *manager,
                                         std::shared_ptr<Input> input) {
    DIMOUSESTATE mouseState;
    if (FAILED(input->Mouse()->GetDeviceState(sizeof(DIMOUSESTATE),
                                              &mouseState))) {
        // retry
        input->Mouse()->Acquire();
    } else {
        auto viewPort = GraphicsManager::Instance().viewport;

        // mouse move vector
        Vector2 vector = Vector2(-mouseState.lX, -mouseState.lY);

        Vector3 origin = manager->camera->GetPosition();

        // convert to spherical coordinates
        double r = origin.Length();
        double phi = acos(origin.y / r);
        double theta = atan2(origin.z, origin.x);

        // rotation
        double deltaTheta = (2 * PI) * (vector.x / viewPort.Width);
        double deltaPhi = (2 * PI) * (vector.y / viewPort.Width);

        theta += deltaTheta;
        phi += deltaPhi;

        // convert to Cartesian coordinates after rotation
        double x = r * sin(phi) * cos(theta);
        double y = r * cos(phi);
        double z = r * sin(phi) * sin(theta);

        Vector3 origin_prime(x, y, z);

        if (0.0f < phi && phi < PI)
            manager->camera->SetPosition(
                Vector3(origin_prime.x, origin_prime.y, origin_prime.z));
                
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
        Vector3 origin = manager->camera->GetPosition();

        Matrix R1(1.0f + wheel, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f + wheel, 0.0f,
                  0.0f, 0.0f, 0.0f, 1.0f + wheel, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

        auto origin_prime = Vector3::Transform(origin, R1);
        manager->camera->SetPosition(
            Vector3(origin_prime.x, origin_prime.y, origin_prime.z));
    }

    return true;
}

bool MessageReceiver::OnWheelDragRequest(PipelineManager *manager,
                                         std::shared_ptr<Input> input,
                                         int mouseX, int mouseY) {
    DIMOUSESTATE mouseState;
    if (FAILED(input->Mouse()->GetDeviceState(sizeof(DIMOUSESTATE),
                                              &mouseState))) {
        // retry
        input->Mouse()->Acquire();
    } else {

        Vector2 current = Vector2(mouseX, mouseY);
        Vector2 before =
            Vector2(mouseX + mouseState.lX, mouseY - mouseState.lY);

        Vector3 cursorNdcCurrent = Vector3(current.x, current.y, 0.0f);
        Vector3 cursorNdcBefore = Vector3(before.x, before.y, 0.0f);

        auto env = common::Env::Instance();

        auto projRow = manager->camera->GetProjection();
        auto viewRow = manager->camera->GetView();

        Matrix inverseProjView = (viewRow * projRow).Invert();

        Vector3 cursorWorldCurrent =
            Vector3::Transform(cursorNdcCurrent, inverseProjView);
        Vector3 cursorWorldBefore =
            Vector3::Transform(cursorNdcBefore, inverseProjView);

        auto move = cursorWorldCurrent - cursorWorldBefore;

        manager->camera->SetLookAt(manager->camera->GetLookAt() + move);
        manager->camera->SetPosition(manager->camera->GetPosition() + move);
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

        auto model = new Model();
        manager->models[model->GetEntityId()] = model;
        GeometryGenerator::ReadFromFile(model, directoryPath, fileName);
        model->SetName(fileName);

        for (const auto &meshData : model->meshes) {
            {
                D3D11_BUFFER_DESC bufferDesc;
                ZeroMemory(&bufferDesc, sizeof(bufferDesc));
                bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
                bufferDesc.ByteWidth =
                    sizeof(dx11::Vertex) *
                    meshData->vertices
                        .size(); // UINT(sizeof(T_VERTEX) * vertices.size());
                bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                bufferDesc.CPUAccessFlags =
                    0; // 0 if no CPU access is necessary.
                bufferDesc.StructureByteStride = sizeof(dx11::Vertex);
                bufferDesc.MiscFlags = 0;

                D3D11_SUBRESOURCE_DATA vertexBufferData = {
                    0}; // MS 예제에서 초기화하는 방식
                vertexBufferData.pSysMem = meshData->vertices.data();
                vertexBufferData.SysMemPitch = 0;
                vertexBufferData.SysMemSlicePitch = 0;

                const HRESULT hr =
                    GraphicsManager::Instance().device->CreateBuffer(
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

                GraphicsManager::Instance().device->CreateBuffer(
                    &bufferDesc, &indexBufferData, &meshData->indexBuffer);
            }

            if (!meshData->textureFilename.empty()) {

                std::cout << meshData->textureFilename << std::endl;

                GraphicsUtil::CreateTexture(
                    GraphicsManager::Instance().device,
                    GraphicsManager::Instance().device_context,
                    meshData->textureFilename, true, meshData->texture,
                    meshData->textureResourceView);
            }

            if (!meshData->albedoTextureFilename.empty()) {

                GraphicsUtil::CreateTexture(
                    GraphicsManager::Instance().device,
                    GraphicsManager::Instance().device_context,
                    meshData->textureFilename, true, meshData->texture,
                    meshData->textureResourceView);
            }

            if (!meshData->emissiveTextureFilename.empty()) {

                GraphicsUtil::CreateTexture(
                    GraphicsManager::Instance().device,
                    GraphicsManager::Instance().device_context,
                    meshData->emissiveTextureFilename, true,
                    meshData->emissiveTexture, meshData->emissiveSRV);
            }

            if (!meshData->normalTextureFilename.empty()) {
                GraphicsUtil::CreateTexture(
                    GraphicsManager::Instance().device,
                    GraphicsManager::Instance().device_context,
                    meshData->normalTextureFilename, true,
                    meshData->normalTexture, meshData->normalSRV);
            }

            if (!meshData->heightTextureFilename.empty()) {
                GraphicsUtil::CreateTexture(
                    GraphicsManager::Instance().device,
                    GraphicsManager::Instance().device_context,
                    meshData->heightTextureFilename, true,
                    meshData->heightTexture, meshData->heightSRV);
            }

            if (!meshData->aoTextureFilename.empty()) {
                GraphicsUtil::CreateTexture(
                    GraphicsManager::Instance().device,
                    GraphicsManager::Instance().device_context,
                    meshData->aoTextureFilename, true, meshData->aoTexture,
                    meshData->aoSRV);
            }

            if (!meshData->metallicTextureFilename.empty()) {
                GraphicsUtil::CreateTexture(
                    GraphicsManager::Instance().device,
                    GraphicsManager::Instance().device_context,
                    meshData->metallicTextureFilename, true,
                    meshData->metallicTexture, meshData->metallicSRV);
            }

            if (!meshData->roughnessTextureFilename.empty()) {
                GraphicsUtil::CreateTexture(
                    GraphicsManager::Instance().device,
                    GraphicsManager::Instance().device_context,
                    meshData->roughnessTextureFilename, true,
                    meshData->roughnessTexture, meshData->roughnessSRV);
            }
        }

    } else {
        // need logger
    }
    return true;
}

bool MessageReceiver::OnSphereLoadRequest(PipelineManager *manager) {

    auto model = new Model();
    model->SetName("sphere");
    manager->models[model->GetEntityId()] = model;

    GeometryGenerator::MakeSphere(model, 1.5f, 15, 13);

    for (auto mesh : model->meshes) {
        GraphicsUtil::CreateTexture(
            GraphicsManager::Instance().device,
            GraphicsManager::Instance().device_context,
            "C:\\Users\\user\\Source\\Engine\\Engine\\Assets\\Textures\\ojwD8."
            "jpg",
            false, mesh->texture, mesh->textureResourceView);

        {
            D3D11_BUFFER_DESC bufferDesc;
            ZeroMemory(&bufferDesc, sizeof(bufferDesc));
            bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
            bufferDesc.ByteWidth =
                sizeof(dx11::Vertex) *
                mesh->vertices
                    .size(); // UINT(sizeof(T_VERTEX) * vertices.size());
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bufferDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
            bufferDesc.StructureByteStride = sizeof(dx11::Vertex);
            bufferDesc.MiscFlags = 0;

            D3D11_SUBRESOURCE_DATA vertexBufferData = {
                0}; // MS 예제에서 초기화하는 방식
            vertexBufferData.pSysMem = mesh->vertices.data();
            vertexBufferData.SysMemPitch = 0;
            vertexBufferData.SysMemSlicePitch = 0;

            const HRESULT hr = GraphicsManager::Instance().device->CreateBuffer(
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

            GraphicsManager::Instance().device->CreateBuffer(
                &bufferDesc, &indexBufferData, &mesh->indexBuffer);
        }
    }

    return true;
}

bool MessageReceiver::OnBoxLoadRequest(PipelineManager *manager) {

    auto model = new Model();
    model->SetName("box");
    manager->models[model->GetEntityId()] = model;

    GeometryGenerator::MakeBox(model);

    for (auto mesh : model->meshes) {
        GraphicsUtil::CreateTexture(
            GraphicsManager::Instance().device,
            GraphicsManager::Instance().device_context,
            "C:\\Users\\user\\Source\\Engine\\Engine\\Assets\\Textures\\crate2_"
            "diffuse.png",
            false, mesh->texture, mesh->textureResourceView);

        {
            D3D11_BUFFER_DESC bufferDesc;
            ZeroMemory(&bufferDesc, sizeof(bufferDesc));
            bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
            bufferDesc.ByteWidth =
                sizeof(dx11::Vertex) *
                mesh->vertices
                    .size(); // UINT(sizeof(T_VERTEX) * vertices.size());
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bufferDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
            bufferDesc.StructureByteStride = sizeof(dx11::Vertex);
            bufferDesc.MiscFlags = 0;

            D3D11_SUBRESOURCE_DATA vertexBufferData = {
                0}; // MS 예제에서 초기화하는 방식
            vertexBufferData.pSysMem = mesh->vertices.data();
            vertexBufferData.SysMemPitch = 0;
            vertexBufferData.SysMemSlicePitch = 0;

            const HRESULT hr = GraphicsManager::Instance().device->CreateBuffer(
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

            GraphicsManager::Instance().device->CreateBuffer(
                &bufferDesc, &indexBufferData, &mesh->indexBuffer);
        }
    }

    return true;
}

bool MessageReceiver::OnCylinderLoadRequest(PipelineManager *manager) {

    auto model = new Model();
    model->SetName("cylinder");
    manager->models[model->GetEntityId()] = model;

    GeometryGenerator::MakeCylinder(model, 5.0f, 5.0f, 15.0f, 30);

    for (auto mesh : model->meshes) {
        GraphicsUtil::CreateTexture(
            GraphicsManager::Instance().device,
            GraphicsManager::Instance().device_context,
            "C:\\Users\\user\\Source\\Engine\\Engine\\Assets\\Textures\\wall."
            "jpg",
            false, mesh->texture, mesh->textureResourceView);

        {
            D3D11_BUFFER_DESC bufferDesc;
            ZeroMemory(&bufferDesc, sizeof(bufferDesc));
            bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
            bufferDesc.ByteWidth =
                sizeof(dx11::Vertex) *
                mesh->vertices
                    .size(); // UINT(sizeof(T_VERTEX) * vertices.size());
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bufferDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
            bufferDesc.StructureByteStride = sizeof(dx11::Vertex);
            bufferDesc.MiscFlags = 0;

            D3D11_SUBRESOURCE_DATA vertexBufferData = {
                0}; // MS 예제에서 초기화하는 방식
            vertexBufferData.pSysMem = mesh->vertices.data();
            vertexBufferData.SysMemPitch = 0;
            vertexBufferData.SysMemSlicePitch = 0;

            const HRESULT hr = GraphicsManager::Instance().device->CreateBuffer(
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

            GraphicsManager::Instance().device->CreateBuffer(
                &bufferDesc, &indexBufferData, &mesh->indexBuffer);
        }
    }

    return true;
}
