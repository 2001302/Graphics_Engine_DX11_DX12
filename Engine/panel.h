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
    int textureToDraw = 0;
};
struct ProjectionSetting {
    float projection_fov_angle_y = 70.0f;
    float near_z = 0.01f;
    float far_z = 100.0f;
};
struct PhysicallyBasedRenderingSetting {
     bool useAlbedoMap = 0;
     bool useNormalMap = 0;
     bool useAOMap = 0;         // Ambient Occlusion
     bool invertNormalMapY = 0; // 16
     bool useMetallicMap = 0;
     bool useRoughnessMap = 0;
     bool useEmissiveMap = 0;

     float metallic = 0.0f;
     float roughness = 0.0f;
};

struct PostProcessSetting {
    float bloom_strength = 0.0f;
    float bloom_exposure = 0.0f;
    float bloom_gamma = 0.0f;
};

enum EnumRenderMode 
{ 
    eLight = 0, 
    eImageBasedLighting = 1,
    ePhysicallyBasedRendering = 2
};

struct GlobalTab {
    bool draw_as_wire_ = false;
    EnumRenderMode render_mode;
    LightSetting light_setting;
    CubeMapSetting cube_map_setting;
    ProjectionSetting projection_setting;
    PhysicallyBasedRenderingSetting pbr_setting;
    PostProcessSetting post_process_setting;
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