#include "ui.h"

using namespace common;

bool IGui::Start() {

    context_ = ImGui::CreateContext();
    ImGui::SetCurrentContext(context_);

    OnStart();

    ImGui::StyleColorsDark();
    RecreateFontAtlas();

    // ImGui_ImplWin32_Init();
    // ImGui_ImplDX12_Init();

    return true;
}

bool IGui::Frame() {
    //ImGui_ImplWin32_NewFrame();
    //ImGui_ImplDX12_NewFrame();

    auto &io = ImGui::GetIO();

    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize,
                        ImGui::GetStyle().WindowBorderSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,
                        ImGui::GetStyle().WindowRounding);

    OnFrame();

    ImGui::PopStyleVar(2);

    ImGui::Render();

    return true;
}

void IGui::RecreateFontAtlas() {
    ImGuiIO &io = ImGui::GetIO();

    IM_DELETE(io.Fonts);

    io.Fonts = IM_NEW(ImFontAtlas);

    ImFontConfig config;
    config.OversampleH = 4;
    config.OversampleV = 4;
    config.PixelSnapH = false;

    io.Fonts->AddFontFromFileTTF("Assets/Fonts/Cuprum-Bold.ttf", 20.0f,
                                 &config);

    io.Fonts->Build();
}
