#include "imgui_manager.h"

using namespace Engine;

bool ImGuiManager::Initialize(HWND mainWindow, Engine::Direct3D *d3d) {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    auto font =
        io.Fonts->AddFontFromFileTTF("../Engine/data/Roboto-Medium.ttf", 16.0f);

    // Windows + DX11
    ImGui_ImplWin32_Init(mainWindow);
    ImGui_ImplDX11_Init(d3d->GetDevice().Get(), d3d->GetDeviceContext().Get());

    // Set style
    SetupImGuiStyle(false, 1.0f);

    ed::Config config;
    config.SettingsFile = "Simple.json";
    m_Context = ed::CreateEditor(&config);

    return true;
}

bool ImGuiManager::Prepare(Env *env) {
    // Start the ImGui frame
    ImGuiIO &io = ImGui::GetIO();
    // Init
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    // Frame
    ImGui::NewFrame();
    ImGui::Begin("Engine");
    ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
    // Viewport
    Direct3D::GetInstance().SetViewPort(ImGui::GetWindowSize().x, 0.0f,
                                        (float)env->screen_width_ -
                                            ImGui::GetWindowSize().x,
                                        (float)env->screen_height_);
    env->aspect_ = ((float)env->screen_width_ - ImGui::GetWindowSize().x) /
                  (float)env->screen_height_;
    return true;
}

bool ImGuiManager::Render(HWND mainWindow) {
    // Add Object
    if (ImGui::Button("Sphere")) {
        SendMessage(mainWindow, WM_SPHERE_LOAD, 0, 0);
    }
    ImGui::SameLine();
    if (ImGui::Button("Box")) {
        SendMessage(mainWindow, WM_BOX_LOAD, 0, 0);
    }
    ImGui::SameLine();
    if (ImGui::Button("Cylinder")) {
        SendMessage(mainWindow, WM_CYLINDER_LOAD, 0, 0);
    }
    ImGui::SameLine();
    if (ImGui::Button("Search")) {
        SendMessage(mainWindow, WM_MODEL_LOAD, 0, 0);
    }
    ImGui::Separator();

    // Framerate
    ImGui::Text("Average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Separator();

    ImGui::BeginTabBar("TabBar");
    if (ImGui::BeginTabItem("Hierarchy")) {
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Inspector")) {
        ImGui::Checkbox("Wire Frame", &m_drawAsWire);
        ImGui::Checkbox("Use Texture", &m_useTexture);
        ImGui::Checkbox("Use BlinnPhong", &m_useBlinnPhong);

        ImGui::Text("Transform");
        ImGui::SliderFloat3("Translation", &m_modelTranslation.x, -2.0f, 2.0f);
        ImGui::SliderFloat("Rotation", &m_modelRotation.y, -3.14f, 3.14f);
        ImGui::SliderFloat3("Scaling", &m_modelScaling.x, 0.1f, 4.0f);

        ImGui::Text("Material");
        ImGui::SliderFloat("Shininess", &m_shininess, 1.0f, 256.0f);
        ImGui::SliderFloat("Diffuse", &m_materialDiffuse, 0.0f, 1.0f);
        ImGui::SliderFloat("Specular", &m_materialSpecular, 0.0f, 1.0f);

        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Light")) {
        if (ImGui::RadioButton("Directional Light", m_lightType == 0)) {
            m_lightType = 0;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Point Light", m_lightType == 1)) {
            m_lightType = 1;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Spot Light", m_lightType == 2)) {
            m_lightType = 2;
        }

        ImGui::SliderFloat3("Position", &m_lightFromGUI.position.x, -5.0f,
                            5.0f);
        ImGui::SliderFloat("Fall Off Start", &m_lightFromGUI.fallOffStart, 0.0f,
                           5.0f);
        ImGui::SliderFloat("Fall Of fEnd", &m_lightFromGUI.fallOffEnd, 0.0f,
                           10.0f);
        ImGui::SliderFloat("Spot Power", &m_lightFromGUI.spotPower, 1.0f,
                           512.0f);

        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();

    ed::SetCurrentEditor(m_Context);
    ed::Begin("My Editor", ImVec2(0.0, 0.0f));
    int uniqueId = 1;
    // Start drawing nodes.
    ed::BeginNode(uniqueId++);
    ImGui::Text("Node A");
    ed::BeginPin(uniqueId++, ed::PinKind::Input);
    ImGui::Text("-> In");
    ed::EndPin();
    ImGui::SameLine();
    ed::BeginPin(uniqueId++, ed::PinKind::Output);
    ImGui::Text("Out ->");
    ed::EndPin();
    ed::EndNode();
    ed::End();
    ed::SetCurrentEditor(nullptr);

    // Rendering
    ImGui::End();
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return true;
}

void ImGuiManager::SetupImGuiStyle(bool styleDark, float alpha) {
    ImGuiStyle &style = ImGui::GetStyle();

    style.Alpha = 1.0f;
    style.WindowRounding = 5.3f;
    style.FrameRounding = 2.3f;
    style.ScrollbarRounding = 0;

    style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] =
        ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] =
        ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] =
        ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive] =
        ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
    style.Colors[ImGuiCol_ResizeGripHovered] =
        ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    style.Colors[ImGuiCol_ResizeGripActive] =
        ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered] =
        ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered] =
        ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);

    if (styleDark) {
        for (int i = 0; i <= ImGuiCol_COUNT; i++) {
            ImVec4 &col = style.Colors[i];
            float H, S, V;
            ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);

            if (S < 0.1f) {
                V = 1.0f - V;
            }
            ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
            if (col.w < 1.00f) {
                col.w *= alpha;
            }
        }
    } else {
        for (int i = 0; i <= ImGuiCol_COUNT; i++) {
            ImVec4 &col = style.Colors[i];
            if (col.w < 1.00f) {
                col.x *= alpha;
                col.y *= alpha;
                col.z *= alpha;
                col.w *= alpha;
            }
        }
    }
}

void ImGuiManager::Shutdown() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}
