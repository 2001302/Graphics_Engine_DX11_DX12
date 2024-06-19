#ifndef _PANEL
#define _PANEL

#include "base_gui.h"
#include "pipeline_manager.h"

namespace ed = ax::NodeEditor;

#ifdef _MSC_VER
#define portable_strcpy strcpy_s
#define portable_sprintf sprintf_s
#else
#define portable_strcpy strcpy
#define portable_sprintf sprintf
#endif

namespace Engine {

struct LinkInfo {
    ed::LinkId Id;
    ed::PinId InputId;
    ed::PinId OutputId;
};

class Panel : public BaseGui {

  public:
    Panel(std::shared_ptr<PipelineManager> pipeline_manager);
    void OnStart() override;
    void OnFrame(float deltaTime) override;

    float shininess_ = 1.0f;
    bool draw_as_wire_ = false;
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

    ed::EditorContext *context_ = nullptr;

  private:
    void DefaultEditor();
    ImVector<LinkInfo> links_;
    int next_link_Id = 100;

    std::shared_ptr<PipelineManager> pipeline_manager_;
    int selected_object_id_ = -99999;
};
} // namespace Engine
#endif