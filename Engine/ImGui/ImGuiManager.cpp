#include "ImGuiManager.h"

using namespace Engine;

bool ImGuiManager::Initialize(HWND hWnd, Engine::Direct3D* d3d)
{
	//Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	auto font = io.Fonts->AddFontFromFileTTF("../Engine/data/Roboto-Medium.ttf", 16.0f);

    //Windows + DX11
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(d3d->GetDevice().Get(), d3d->GetDeviceContext().Get());

    //Set window handle
	WindowHandler::GetInstance().SetHandle(hWnd);

    //Set style
    SetupImGuiStyle(false, 1.0f);

	return true;
}

bool ImGuiManager::Prepare(int screenWidth, int screenHeight)
{
	//Start the ImGui frame
	ImGuiIO& io = ImGui::GetIO();

	//Frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
    Direct3D::GetInstance().SetViewPort(EnumViewType::eScene, ImGui::GetWindowSize().x, 0.0f, (float)screenWidth - ImGui::GetWindowSize().x, (float)screenHeight);

	return true;
}

bool ImGuiManager::Render()
{
    //Custom UI
    if (ImGui::Button("Asset"))
    {
        SendMessage(WindowHandler::GetInstance().GetHandle(), WM_MODEL_LOAD, 0, 0);
    }

	//Rendering
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return true;
}

void ImGuiManager::SetupImGuiStyle(bool styleDark, float alpha)
{
    ImGuiStyle& style = ImGui::GetStyle();

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
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);

    if (styleDark)
    {
        for (int i = 0; i <= ImGuiCol_COUNT; i++)
        {
            ImVec4& col = style.Colors[i];
            float H, S, V;
            ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);

            if (S < 0.1f)
            {
                V = 1.0f - V;
            }
            ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
            if (col.w < 1.00f)
            {
                col.w *= alpha;
            }
        }
    }
    else
    {
        for (int i = 0; i <= ImGuiCol_COUNT; i++)
        {
            ImVec4& col = style.Colors[i];
            if (col.w < 1.00f)
            {
                col.x *= alpha;
                col.y *= alpha;
                col.z *= alpha;
                col.w *= alpha;
            }
        }
    }
}

void ImGuiManager::Shutdown()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}
