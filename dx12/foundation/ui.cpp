#include "ui.h"

using namespace foundation;

bool IGui::Start() {

    context_ = ImGui::CreateContext();
    ImGui::SetCurrentContext(context_);

    OnStart();

    ImGui::StyleColorsDark();
    RecreateFontAtlas();

    //ImGui_ImplWin32_Init();
    //ImGui_ImplDX12_Init();

    return true;
}

bool IGui::Frame() {

    auto &io = ImGui::GetIO();

    ImGui_ImplWin32_NewFrame();
    ImGui_ImplDX12_NewFrame();

    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize,
                        ImGui::GetStyle().WindowBorderSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,
                        ImGui::GetStyle().WindowRounding);

    OnFrame();

    ImGui::PopStyleVar(2);

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(),
                                  dx12::GpuCore::Instance().commandList.Get());

    return true;
}

void IGui::Shutdown() {
    ImGui_ImplDX12_Shutdown();
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

    io.Fonts->AddFontFromFileTTF("Assets/Fonts/Cuprum-Bold.ttf", 20.0f,
                                 &config);

    io.Fonts->Build();
}
