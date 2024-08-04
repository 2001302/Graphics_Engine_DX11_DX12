#include "ui.h"

using namespace common;

bool IGui::Initialize() {

    context_ = ImGui::CreateContext();
    ImGui::SetCurrentContext(context_);

    ImGui_ImplWin32_Init(common::Env::Instance().main_window);
    ImGui_ImplDX11_Init(
        engine::GraphicsManager::Instance().device.Get(),
        engine::GraphicsManager::Instance().device_context.Get());

    ImGui::StyleColorsDark();
    RecreateFontAtlas();

    OnStart();
    Frame();

    return true;
}

bool IGui::Frame() {

    auto &io = ImGui::GetIO();

    ImGui_ImplWin32_NewFrame();
    ImGui_ImplDX11_NewFrame();

    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize,
                        ImGui::GetStyle().WindowBorderSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,
                        ImGui::GetStyle().WindowRounding);

    OnFrame();

    ImGui::PopStyleVar(2);

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return true;
}

void IGui::Shutdown() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void IGui::RecreateFontAtlas() {
    ImGuiIO &io = ImGui::GetIO();

    IM_DELETE(io.Fonts);

    io.Fonts = IM_NEW(ImFontAtlas);

    ImFontConfig config;
    config.OversampleH = 4;
    config.OversampleV = 4;
    config.PixelSnapH = false;

    io.Fonts->AddFontFromFileTTF(
        "Assets/Fonts/Cuprum-Bold.ttf", 20.0f, &config);

    io.Fonts->Build();
}
