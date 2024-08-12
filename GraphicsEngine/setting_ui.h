#ifndef _PANEL
#define _PANEL

#include "graphics_manager.h"
#include "message.h"
#include "model.h"
#include "ui.h"
#include "info.h"
#include <map>
#include <string>

namespace ed = ax::NodeEditor;

namespace engine {

class SettingUi : public common::IGui {
    enum EnumViewType {
        e3dViewport = 0,
        eNodeEditor = 1,
    };

  public:
    void PushNodeItem(common::INode *node);
    void ClearNodeItem();
    void PushInfoItem(common::IInfo *node);
    void ClearInfoItem();

  private:
    void OnStart() override;
    void OnFrame() override;
    void TopBar();
    void MainView();
    void LeftPanel();
    void Hierarchy();
    void NodeEditor();

    float screen_width;
    float screen_hieght;

    std::vector<common::INode *> node_items;
    std::vector<common::IInfo *> info_items;

    ed::EditorContext *context_ = nullptr;

    ImVector<common::INode::LinkInfo> links_;
    int next_link_Id = 100;
    int unique_id = 1;
    int unique_pos_x = 0;

    EnumViewType view_type;
    const float offset_top = 35.0f;
    const float left_panel_width = 300.0f;

    common::INode selected_object;
};
} // namespace engine
#endif
