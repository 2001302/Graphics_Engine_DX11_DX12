#include "message_receiver.h"
#include "renderer.h"

namespace engine {
using namespace DirectX::SimpleMath;

bool MessageReceiver::OnMouseRightDragRequest(RenderingBlock *manager,
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

bool MessageReceiver::OnMouseWheelRequest(RenderingBlock *manager,
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

bool MessageReceiver::OnMouseWheelDragRequest(RenderingBlock *manager,
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

        manager->camera->SetPosition(manager->camera->GetPosition() + move);
        manager->camera->SetLookAt(manager->camera->GetLookAt() + move);
    }

    return true;
}

bool MessageReceiver::OnModelLoadRequest(RenderingBlock *manager,
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

        Renderer *renderer = new Renderer(
            GraphicsManager::Instance().device,
                         GraphicsManager::Instance().device_context,
                         directoryPath, fileName);

        renderer->UpdateConstantBuffers(
            GraphicsManager::Instance().device,
            GraphicsManager::Instance().device_context);

        auto obj = std::make_shared<Model>();
        obj->AddComponent(EnumComponentType::eRenderer, renderer);
        obj->SetName(fileName);

        manager->models.insert({obj->GetEntityId(), obj});

    } else {
        // need logger
    }
    return true;
}

bool MessageReceiver::OnSphereLoadRequest(RenderingBlock *manager) {

    MeshData mesh = GeometryGenerator::MakeSphere(0.2f, 200, 200);
    Renderer *renderer = new Renderer(
        GraphicsManager::Instance().device,
        GraphicsManager::Instance().device_context, std::vector{mesh});
    renderer->UpdateConstantBuffers(GraphicsManager::Instance().device,
                                    GraphicsManager::Instance().device_context);

    auto obj = std::make_shared<Model>();
    obj->AddComponent(EnumComponentType::eRenderer, renderer);
    obj->SetName("sphere");

    manager->models.insert({obj->GetEntityId(), obj});

    return true;
}

bool MessageReceiver::OnBoxLoadRequest(RenderingBlock *manager) {

    MeshData mesh = GeometryGenerator::MakeBox(1.0f);
    Renderer *renderer = new Renderer(
        GraphicsManager::Instance().device,
        GraphicsManager::Instance().device_context, std::vector{mesh});
    renderer->UpdateConstantBuffers(GraphicsManager::Instance().device,
                                    GraphicsManager::Instance().device_context);

    auto obj = std::make_shared<Model>();
    obj->AddComponent(EnumComponentType::eRenderer, renderer);
    obj->SetName("box");

    manager->models.insert({obj->GetEntityId(), obj});

    return true;
}

bool MessageReceiver::OnCylinderLoadRequest(RenderingBlock *manager) {

    MeshData mesh = GeometryGenerator::MakeCylinder(5.0f, 5.0f, 15.0f, 30);
    Renderer *renderer = new Renderer(
        GraphicsManager::Instance().device,
        GraphicsManager::Instance().device_context, std::vector{mesh});
    renderer->UpdateConstantBuffers(GraphicsManager::Instance().device,
                                    GraphicsManager::Instance().device_context);

    auto obj = std::make_shared<Model>();
    obj->AddComponent(EnumComponentType::eRenderer, renderer);
    obj->SetName("box");

    manager->models.insert({obj->GetEntityId(), obj});

    return true;
}
} // namespace engine