#ifndef _PANEL
#define _PANEL

#include "ui.h"
#include "message.h"
#include "model.h"
#include <string>
#include <map>

namespace ed = ax::NodeEditor;

namespace common {

class SettingUi : public IGui {
    enum EnumViewType 
    {
        e3dViewport = 0,
        eNodeEditor = 1,
    };

  public:
    void OnStart() override;
    void OnFrame() override;
    void PushNodeItem(INode *node);
    void ClearNodeItem();
    void PushPanelItem(INode *node);
    void ClearPanelItem();

    //int SelectedId();

  private:
    void TopBar();
    void MainView();

    void LeftPanel();
    void Hierarchy();

    void NodeEditor();

    ed::EditorContext *context_ = nullptr;

    ImVector<INode::LinkInfo> links_;
    int next_link_Id = 100;

    int selected_object_id_ = -99999;
    int unique_id = 1;
    int unique_pos_x = 0;
    
    std::vector<INode *> node_items;
    std::vector<INode *> panel_items;

    const float offset_top = 35.0f;
    const float left_panel_width = 300.0f;
    EnumViewType view_type;

    float screen_width;
    float screen_hieght;
};
} // namespace common
#endif
