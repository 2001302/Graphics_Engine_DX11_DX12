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
class Panel : public BaseGui {

    struct LinkInfo {
        ed::LinkId Id;
        ed::PinId InputId;
        ed::PinId OutputId;
    };

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
    ImVector<LinkInfo> links_; // List of live links. It is dynamic unless you
                               // want to create read-only view over nodes.
    int next_link_Id =
        100; // Counter to help generate link ids. In real application this will
             // probably based on pointer to user data structure.

  private:
    std::shared_ptr<PipelineManager> pipeline_manager_;
    int selected_object_id_;
    // selected object의 graph를 참조하여 render 수행, graph 의 node를 추가
};
} // namespace Engine
#endif