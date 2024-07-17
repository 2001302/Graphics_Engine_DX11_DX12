#ifndef _PANEL
#define _PANEL

#include "gui_base.h"
#include "light.h"
#include "message.h"
#include <string>
#include <map>

namespace ed = ax::NodeEditor;

namespace common {

struct CommonSetting {
    bool draw_as_wire_ = false;
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
    bool useTexture;
    bool useNormalMap;
    bool useAOMap;
    bool reverseNormalMapY;
};

struct TabInfo {
    CommonSetting common;
    LightSetting light;
    CubeMapSetting cube_map;
    FilterSetting filter;
    GroundSetting ground;
};
class SettingUi : public IGui {

  public:
    void OnStart() override;
    void OnFrame() override;
    TabInfo Tab() { return tab; }
    int SelectedId();
    void PushNode(INodeUi *node);
    void ClearNode();

    void StyleSetting();
    void FrameRate();
    void MenuBar();
    void NodeEditor();
    void TabBar(std::map<int, INodeUi *> node_map);

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
