#include "base_gui.h"

using namespace Engine;

#ifdef _MSC_VER
#define portable_strcpy strcpy_s
#define portable_sprintf sprintf_s
#else
#define portable_strcpy strcpy
#define portable_sprintf sprintf
#endif

ImGuiWindowFlags GetWindowFlags() {
    return ImGuiWindowFlags_NoTitleBar |
           ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove |
           ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
}

bool BaseGui::Initialize(HWND main_window, Env *env) {
    main_window_ = main_window;
    env_ = env;
    // Setup Dear ImGui context
    context_ = ImGui::CreateContext();
    ImGui::SetCurrentContext(context_);

    ImGui_ImplWin32_Init(main_window_);
    ImGui_ImplDX11_Init(Direct3D::GetInstance().GetDevice().Get(),
                        Direct3D::GetInstance().GetDeviceContext().Get());

    ImGui::StyleColorsDark();
    RecreateFontAtlas();

    OnStart();

    Frame();

    return true;
}

bool BaseGui::Frame() {

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

    if (ImGui::GetCurrentWindow()) {
        Direct3D::GetInstance().SetViewPort(ImGui::GetWindowSize().x, 0.0f,
                                            (float)env_->screen_width_ -
                                                ImGui::GetWindowSize().x,
                                            (float)env_->screen_height_);
        env_->aspect_ =
            ((float)env_->screen_width_ - ImGui::GetWindowSize().x) /
            (float)env_->screen_height_;
    }

    OnFrame(io.DeltaTime);

    ImGui::PopStyleVar(2);
    ImGui::End();
    ImGui::PopStyleVar(2);

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return true;
}

void BaseGui::Shutdown() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void BaseGui::RecreateFontAtlas() {
    ImGuiIO &io = ImGui::GetIO();

    IM_DELETE(io.Fonts);

    io.Fonts = IM_NEW(ImFontAtlas);

    ImFontConfig config;
    config.OversampleH = 4;
    config.OversampleV = 4;
    config.PixelSnapH = false;

    default_font_ = io.Fonts->AddFontFromFileTTF(
        "../Engine/data/Play-Regular.ttf", 18.0f, &config);
    header_font = io.Fonts->AddFontFromFileTTF("../Engine/data/Cuprum-Bold.ttf",
                                               20.0f, &config);

    io.Fonts->Build();
}