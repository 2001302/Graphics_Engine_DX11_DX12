#include "ui.h"

using namespace common;

bool IGui::Start() {

    context_ = ImGui::CreateContext();
    ImGui::SetCurrentContext(context_);

    OnStart();

    ImGui::StyleColorsDark();
    RecreateFontAtlas();

    return true;
}

bool IGui::Frame() {
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

std::filesystem::path GetExecutablePath() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return std::filesystem::path(path).parent_path();
}

void IGui::RecreateFontAtlas() {
    ImGuiIO &io = ImGui::GetIO();

    IM_DELETE(io.Fonts);

    io.Fonts = IM_NEW(ImFontAtlas);

    ImFontConfig config;
    config.OversampleH = 4;
    config.OversampleV = 4;
    config.PixelSnapH = false;

    std::filesystem::path font_path = GetExecutablePath() / "Cuprum-Bold.ttf";
    io.Fonts->AddFontFromFileTTF(font_path.string().c_str(), 20.0f, &config);

    io.Fonts->Build();
}
