#ifndef _PANEL
#define _PANEL

#include "gui_base.h"
#include "light.h"
#include "message.h"
#include <string>

namespace ed = ax::NodeEditor;

namespace common {

enum EnumRenderMode {
    eLight = 0,
    eImageBasedLighting = 1,
    ePhysicallyBasedRendering = 2
};

struct ProjectionSetting {
    float projection_fov_angle_y = 70.0f;
    float near_z = 0.01f;
    float far_z = 100.0f;
};

struct CommonSetting {
    bool draw_as_wire_ = false;
    EnumRenderMode render_mode;
};

struct LightSetting {
    bool use_blinn_phong;
    int light_type;
    dx11::Light light_from_gui;
};

struct CubeMapSetting {
    int textureToDraw;
    float mipLevel;
};

struct PostProcessSetting {
    float bloom_strength;
    float bloom_exposure;
    float bloom_gamma;
};

struct GroundSetting {
    bool useHeightMap;
    float heightScale;
    float ambient;                 
    float shininess;             
    float diffuse;                
    float specular;            
    float fresnelR0;  
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

struct GlobalTab {
    CommonSetting common_setting;
    LightSetting light_setting;
    CubeMapSetting cube_map_setting;
    PostProcessSetting post_process_setting;
    GroundSetting ground_setting;
    ProjectionSetting projection_setting;
    PhysicallyBasedRenderingSetting pbr_setting;
};
class SettingUi : public IGui {

  public:
    void OnStart() override;
    void OnFrame() override;
    GlobalTab GetGlobalTab() { return global_setting; }

  private:
    void StyleSetting();
    void FrameRate();
    void MenuBar();
    void NodeEditor();
    void TabBar();

    ed::EditorContext *context_ = nullptr;
    GlobalTab global_setting;
    int selected_object_id_ = -99999;
};
} // namespace common
#endif
