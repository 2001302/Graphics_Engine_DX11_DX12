#ifndef _PANEL
#define _PANEL

#include "gui_base.h"
#include "message.h"
#include <string>
#include <map>

namespace ed = ax::NodeEditor;

namespace common {

class SettingUi : public IGui {

  public:
    void OnStart() override;
    void OnFrame() override;
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

    ImVector<INodeUi::LinkInfo> links_;
    int next_link_Id = 100;

    int selected_object_id_ = -99999;
    std::vector<INodeUi *> nodes;
    int unique_id = 1;
    int unique_pos_x = 0;
};
} // namespace common
#endif
