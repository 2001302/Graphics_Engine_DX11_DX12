#include "../Direct3D.h"
#include "../IDataBlock.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

namespace Engine
{
	class ImGuiManager
	{
	public:
		ImGuiManager() {};
		ImGuiManager(const ImGuiManager&) {};
		~ImGuiManager() {};

		bool Initialize(HWND mainWindow, Engine::Direct3D* d3d);
		bool Prepare(Env* aspect);
		bool Render(HWND mainWindow);
		void Shutdown();

	private:
		void SetupImGuiStyle(bool styleDark, float alpha);

		bool m_usePerspectiveProjection = true;
		DirectX::SimpleMath::Vector3 m_modelTranslation = DirectX::SimpleMath::Vector3(0.0f);
		DirectX::SimpleMath::Vector3 m_modelRotation = DirectX::SimpleMath::Vector3(0.0f);
		DirectX::SimpleMath::Vector3 m_modelScaling = DirectX::SimpleMath::Vector3(0.5f);
		float m_viewRot = 0.0f;

		float m_projFovAngleY = 70.0f;
		float m_nearZ = 0.01f;
		float m_farZ = 100.0f;

		int m_lightType = 0;
		LightInfo m_lightFromGUI;
		float m_materialDiffuse = 1.0f;
		float m_materialSpecular = 1.0f;
	};
}