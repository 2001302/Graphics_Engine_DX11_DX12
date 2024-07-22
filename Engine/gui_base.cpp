#include "gui_base.h"

using namespace common;

ImGuiWindowFlags GetWindowFlags() {
    return ImGuiWindowFlags_NoTitleBar |
           ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove;
}

bool IGui::Initialize() {
    // Setup Dear ImGui context
    context_ = ImGui::CreateContext();
    ImGui::SetCurrentContext(context_);

    ImGui_ImplWin32_Init(Env::Instance().main_window);
    ImGui_ImplDX11_Init(
        engine::GraphicsManager::Instance().device.Get(),
        engine::GraphicsManager::Instance().device_context.Get());

    ImGui::StyleColorsDark();
    RecreateFontAtlas();

    OnStart();

    FrameBegin();
    FrameEnd();

    return true;
}

bool IGui::FrameBegin() {

    auto &io = ImGui::GetIO();

    ImGui_ImplWin32_NewFrame();
    ImGui_ImplDX11_NewFrame();

    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));

    const auto windowBorderSize = ImGui::GetStyle().WindowBorderSize;
    const auto windowRounding = ImGui::GetStyle().WindowRounding;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("Menu", nullptr, GetWindowFlags());
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, windowBorderSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, windowRounding);

    return true;
}

bool IGui::FrameEnd() {

    OnFrame();

    ImGui::PopStyleVar(2);
    ImGui::End();
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

    default_font_ = io.Fonts->AddFontFromFileTTF(
        "Assets/Fonts/Play-Regular.ttf", 18.0f, &config);
    header_font = io.Fonts->AddFontFromFileTTF("Assets/Fonts/Cuprum-Bold.ttf",
                                               20.0f, &config);

    io.Fonts->Build();
}
