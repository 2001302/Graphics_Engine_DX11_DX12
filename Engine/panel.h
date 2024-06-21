#ifndef _PANEL
#define _PANEL

#include "base_gui.h"
#include "pipeline_manager.h"

namespace ed = ax::NodeEditor;

namespace Engine {

struct LightSetting {
    bool use_blinn_phong = false;
    int light_type = 0;
    Light light_from_gui;
};
struct CubeMapSetting {
    bool use_image_based_lighting = false;
};
struct ProjectionSetting {
    float projection_fov_angle_y = 70.0f;
    float near_z = 0.01f;
    float far_z = 100.0f;
};
enum EnumRenderMode 
{ 
    eLight = 0, 
    eCubeMapping = 1 
};

struct GlobalTab {
    bool draw_as_wire_ = false;
    EnumRenderMode render_mode;
    LightSetting light_setting;
    CubeMapSetting cube_map_setting;
    ProjectionSetting projection_setting;
};
class Panel : public BaseGui {

  public:
    Panel(std::shared_ptr<PipelineManager> pipeline_manager);
    void OnStart() override;
    void OnFrame(float deltaTime) override;
    GlobalTab GetGlobalTab() { return global_setting; }

  private:
    void StyleSetting();
    void FrameRate();
    void MenuBar();
    void NodeEditor();
    void TabBar();

    ed::EditorContext *context_ = nullptr;
    std::shared_ptr<PipelineManager> pipeline_manager_;
    GlobalTab global_setting;
    int selected_object_id_ = -99999;
};
} // namespace Engine
#endif