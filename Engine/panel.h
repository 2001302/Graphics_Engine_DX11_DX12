#ifndef _PANEL
#define _PANEL

#include "base_gui.h"
#include "pipeline_manager.h"

namespace ed = ax::NodeEditor;

namespace Engine {

class Panel : public BaseGui {

  public:
    Panel(std::shared_ptr<PipelineManager> pipeline_manager);
    void OnStart() override;
    void OnFrame(float deltaTime) override;

    bool draw_as_wire_ = false;
    float projection_fov_angle_y = 70.0f;
    float near_z = 0.01f;
    float far_z = 100.0f;
    int light_type = 0;
    Light light_from_gui;

  private:
    void StyleSetting();
    void FrameRate();
    void MenuBar();
    void NodeEditor();
    void TabBar();
    ed::EditorContext *context_ = nullptr;
    std::shared_ptr<PipelineManager> pipeline_manager_;
    int selected_object_id_ = -99999;
};
} // namespace Engine
#endif