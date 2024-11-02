#ifndef _PANEL
#define _PANEL

#include "env.h"
#include "info.h"
#include "message.h"
#include "node.h"
#include "ui.h"
#include <map>
#include <string>
#include <vector>

namespace ed = ax::NodeEditor;

namespace common {

struct Rect {
    float left, right, top, bottom;
    constexpr Rect() : left(0.0f), right(0.0f), top(0.0f), bottom(0.0f) {}
    constexpr Rect(float _left, float _top, float _right, float _bottom)
        : left(_left), top(_top), right(_right), bottom(_bottom) {}
};

class SettingUi : public IGui {
  public:
    enum EnumViewType {
        eEdit = 0,
        eGame = 1,
    };

    void PushNodeItem(INode *node);
    void ClearNodeItem();
    void PushInfoItem(IInfo *node);
    void ClearInfoItem();

    EnumViewType GetViewType();
    Rect GetRect();

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
} // namespace common
#endif
