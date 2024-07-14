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
    int light_type;
    dx11::Light light_from_gui;
};

struct CubeMapSetting {
    int textureToDraw;
    float mipLevel;
};

struct FilterSetting {
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

struct TabInfo {
    CommonSetting common;
    LightSetting light;
    CubeMapSetting cube_map;
    FilterSetting filter;
    GroundSetting ground;
    ProjectionSetting projection;
    PhysicallyBasedRenderingSetting pbr;
};
class SettingUi : public IGui {

  public:
    void OnStart() override;
    void OnFrame() override;
    TabInfo Tab() { return tab; }
    int SelectedId() {
        if (selected_object_id_ == -99999)
            return 0;
        else {
            return selected_object_id_;
        }
    }

    void StyleSetting();
    void FrameRate();
    void MenuBar();
    void NodeEditor();
    void TabBar(std::unordered_map<int, std::shared_ptr<INodeUi>> node_map);

    void PushNode(INodeUi *node) {
        node->uniqueId = unique_id;
        node->position = ImVec2(unique_pos_x, 0);

        nodes.push_back(node);
        unique_id = unique_id + 10;
        unique_pos_x = unique_pos_x + 500;
    };
    void ClearNode() {
        unique_id = 1;
        unique_pos_x = 0;
        nodes.clear();
    };

  private:

    ed::EditorContext *context_ = nullptr;
    TabInfo tab;

    ImVector<INodeUi::LinkInfo> links_;
    int next_link_Id = 100;

    int selected_object_id_ = -99999;
    std::vector<INodeUi *> nodes;
    int unique_id = 1;
    int unique_pos_x = 0;
};
} // namespace common
#endif
