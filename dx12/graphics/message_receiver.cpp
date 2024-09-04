#include "message_receiver.h"
//#include "mesh_renderer.h"

namespace graphics {
using namespace DirectX::SimpleMath;

bool MessageReceiver::OnWindowSizeRequest(foundation::SettingUi *gui, int size_x,
                                          int size_y) {

    foundation::Env::Instance().screen_width = size_x;
    foundation::Env::Instance().screen_height = size_y;

    if (foundation::Env::Instance().screen_width &&
        foundation::Env::Instance().screen_height) {
        //if (dx11::GpuCore::Instance().swap_chain) {

        //    gui->Shutdown();

        //    dx11::GpuCore::Instance().back_buffer_RTV.Reset();
        //    dx11::GpuCore::Instance().swap_chain->ResizeBuffers(
        //        0, size_x, size_y, DXGI_FORMAT_UNKNOWN, 0);

        //    dx11::GpuCore::Instance().CreateBuffer();
        //    dx11::GpuCore::Instance().SetMainViewport();

        //    gui->Start();
        //}
    }
    return true;
}
bool MessageReceiver::OnMouseDownRequest(std::shared_ptr<foundation::Input> input,
                                         int mouseX, int mouseY) {
    input->SetMouse(mouseX, mouseY);
    return true;
}

bool MessageReceiver::OnMouseRightDragRequest(
    RenderTargetObject *manager, std::shared_ptr<foundation::Input> input, int mouseX,
    int mouseY) {

    Vector2 move = Vector2(input->MouseX() - mouseX, input->MouseY() - mouseY);
    move = move / -1000.0f;

    Vector3 relative_position =
        manager->camera->GetPosition() - manager->camera->GetLookAt();
    Matrix rotation_matrix =
        Matrix::CreateRotationX(move.y) * Matrix::CreateRotationY(move.x);
    Vector3 rotated_position =
        Vector3::Transform(relative_position, rotation_matrix);
    Vector3 position = rotated_position + manager->camera->GetLookAt();

    manager->camera->SetPosition(Vector3(position.x, position.y, position.z));

    input->SetMouse(mouseX, mouseY);
    return true;
}

bool MessageReceiver::OnMouseWheelRequest(RenderTargetObject *manager,
                                          std::shared_ptr<foundation::Input> input,
                                          int wheel) {

    auto look_at = manager->camera->GetLookAt();
    auto position = manager->camera->GetPosition();

    auto direction = look_at - position;
    if (direction.Length() >= 1.0f || wheel < 0) {
        direction.Normalize();

        auto move = wheel * direction / 1000.0f;
        auto new_position = position + move;

        manager->camera->SetPosition(new_position);
    }
    return true;
}

bool MessageReceiver::OnMouseWheelDragRequest(
    RenderTargetObject *manager, std::shared_ptr<foundation::Input> input, int mouseX,
    int mouseY) {
    auto offsetX = input->MouseX() - mouseX;
    auto offsetY = input->MouseY() - mouseY;

    Vector3 cursorNdcCurrent = Vector3(mouseX, mouseY, 0.0f);
    Vector3 cursorNdcBefore = Vector3(mouseX - offsetX, mouseY + offsetY, 0.0f);

    Matrix inverseProjView =
        (manager->camera->GetView() * manager->camera->GetProjection())
            .Invert();

    Vector3 cursorWorldCurrent =
        Vector3::Transform(cursorNdcCurrent, inverseProjView);
    Vector3 cursorWorldBefore =
        Vector3::Transform(cursorNdcBefore, inverseProjView);

    auto move = cursorWorldCurrent - cursorWorldBefore;

    manager->camera->SetPosition(manager->camera->GetPosition() + move);
    manager->camera->SetLookAt(manager->camera->GetLookAt() + move);

    input->SetMouse(mouseX, mouseY);
    return true;
}

bool MessageReceiver::OnModelLoadRequest(RenderTargetObject *manager,
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

    //// show file explorer
    //if (GetOpenFileName(&ofn)) {

    //    std::string fullPath = ToString(ofn.lpstrFile);
    //    size_t lastSlash = fullPath.find_last_of('\\');
    //    std::string fileName = fullPath.substr(lastSlash + 1);
    //    std::string directoryPath = fullPath.substr(0, lastSlash) + "\\";

    //    auto renderer = std::make_shared<MeshRenderer>(directoryPath, fileName);

    //    renderer->UpdateConstantBuffers();

    //    auto obj = std::make_shared<Model>();
    //    obj->AddComponent(EnumComponentType::eRenderer, renderer);
    //    obj->SetName(fileName);

    //    manager->objects.insert({obj->GetEntityId(), obj});

    //} else {
    //    // need logger
    //}
    return true;
}

bool MessageReceiver::OnSphereLoadRequest(RenderTargetObject *manager) {

    //MeshData mesh = GeometryGenerator::MakeSphere(0.2f, 200, 200);
    //auto renderer = std::make_shared<MeshRenderer>(std::vector{mesh});
    //renderer->UpdateConstantBuffers();

    //auto obj = std::make_shared<Model>();
    //obj->AddComponent(EnumComponentType::eRenderer, renderer);
    //obj->SetName("sphere");

    //manager->objects.insert({obj->GetEntityId(), obj});

    return true;
}

bool MessageReceiver::OnBoxLoadRequest(RenderTargetObject *manager) {

    //MeshData mesh = GeometryGenerator::MakeBox(1.0f);
    //auto renderer = std::make_shared<MeshRenderer>(std::vector{mesh});
    //renderer->UpdateConstantBuffers();

    //auto obj = std::make_shared<Model>();
    //obj->AddComponent(EnumComponentType::eRenderer, renderer);
    //obj->SetName("box");

    //manager->objects.insert({obj->GetEntityId(), obj});

    return true;
}

bool MessageReceiver::OnCylinderLoadRequest(RenderTargetObject *manager) {

    //MeshData mesh = GeometryGenerator::MakeCylinder(5.0f, 5.0f, 15.0f, 30);
    //auto renderer = std::make_shared<MeshRenderer>(std::vector{mesh});
    //renderer->UpdateConstantBuffers();

    //auto obj = std::make_shared<Model>();
    //obj->AddComponent(EnumComponentType::eRenderer, renderer);
    //obj->SetName("box");

    //manager->objects.insert({obj->GetEntityId(), obj});

    return true;
}
} // namespace core
