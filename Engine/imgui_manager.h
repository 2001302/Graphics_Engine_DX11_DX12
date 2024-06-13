#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_node_editor.h"
#include "ImGui/imgui_internal.h"
#include "dataBlock.h"
#include "direct3D.h"

namespace ed = ax::NodeEditor;

namespace Engine {
class ImGuiManager : public IDataBlock {

    struct LinkInfo {
        ed::LinkId Id;
        ed::PinId InputId;
        ed::PinId OutputId;
    };

  public:
    ImGuiManager(){};
    ImGuiManager(const ImGuiManager &){};
    ~ImGuiManager(){};

    bool Initialize(HWND mainWindow, Engine::Direct3D *d3d);
    bool Frame(HWND mainWindow, Env *env);

    bool Prepare(Env *env);
    void Shutdown();

    float m_shininess = 1.0f;
    bool m_drawAsWire = false;
    bool m_useBlinnPhong = false;
    bool m_useTexture = false;

    bool m_usePerspectiveProjection = true;
    DirectX::SimpleMath::Vector3 m_modelTranslation =
        DirectX::SimpleMath::Vector3(0.0f);
    DirectX::SimpleMath::Vector3 m_modelRotation =
        DirectX::SimpleMath::Vector3(0.0f);
    DirectX::SimpleMath::Vector3 m_modelScaling =
        DirectX::SimpleMath::Vector3(1.0f);

    float m_projFovAngleY = 70.0f;
    float m_nearZ = 0.01f;
    float m_farZ = 100.0f;

    int m_lightType = 0;
    Light m_lightFromGUI;
    float m_materialDiffuse = 1.0f;
    float m_materialSpecular = 1.0f;

    ImVector<LinkInfo> m_Links; // List of live links. It is dynamic unless you
                                // want to create read-only view over nodes.
    int m_NextLinkId =
        100; // Counter to help generate link ids. In real application this will
             // probably based on pointer to user data structure.

    ed::EditorContext *m_Context = nullptr;

    ImFont *m_DefaultFont = nullptr;
    ImFont *m_HeaderFont = nullptr;

  private:
    void SetupImGuiStyle(bool styleDark, float alpha);
};
} // namespace Engine