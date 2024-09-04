#ifndef _PANEL
#define _PANEL

#include "message.h"
#include "node.h"
#include "ui.h"
#include "info.h"
#include "env.h"
#include <map>
#include <string>
#include <vector>

namespace ed = ax::NodeEditor;

namespace foundation {

class SettingUi : public IGui {
    enum EnumViewType {
        e3dViewport = 0,
        eNodeEditor = 1,
    };

  public:
    void PushNodeItem(INode *node);
    void ClearNodeItem();
    void PushInfoItem(IInfo *node);
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

    std::vector<INode *> node_items;
    std::vector<IInfo *> info_items;

    ed::EditorContext *context_ = nullptr;

    ImVector<INode::LinkInfo> links_;
    int next_link_Id = 100;
    int unique_id = 1;
    int unique_pos_x = 0;

    EnumViewType view_type;
    const float offset_top = 35.0f;
    const float left_panel_width = 300.0f;

    INode selected_object;
};
} // namespace engine
#endif
