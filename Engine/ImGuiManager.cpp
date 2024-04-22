#include "ImGuiManager.h"

bool ImGuiManager::Initialize(HWND hWnd, Engine::D3DClass* d3d)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	auto font = io.Fonts->AddFontFromFileTTF("../Engine/data/Roboto-Black.ttf", 16.0f);

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(d3d->GetDevice(), d3d->GetDeviceContext());

	return true;
}

bool ImGuiManager::Prepare()
{
	// (Your code process and dispatch Win32 messages)
	// Start the ImGui frame
	ImGuiIO& io = ImGui::GetIO();

	//Frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	////Font
	//ImGui::PushFont(io.Fonts->Fonts.back());
	//ImGui::Text("Hello with font size 16!");
	//ImGui::PopFont();

	ShowWindow();

	return true;
}

bool ImGuiManager::Render()
{
	// Rendering
	// (Your code clears your framebuffer, renders your other stuff etc.)
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	// (Your code calls swapchain's Present() function)
	return true;
}

void ImGuiManager::Shutdown()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}