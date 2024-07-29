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

  public:
    void OnStart() override;
    void OnFrame(IDataBlock *dataBlock) override;
    int SelectedId();
    void PushNode(INode *node);
    void ClearNode();

    void StyleSetting();
    void FrameRate();
    void MenuBar();
    void NodeEditor();
    void TabBar(IDataBlock *dataBlock);

  private:
    ed::EditorContext *context_ = nullptr;

    ImVector<INode::LinkInfo> links_;
    int next_link_Id = 100;

    int selected_object_id_ = -99999;
    std::vector<INode *> nodes;
    int unique_id = 1;
    int unique_pos_x = 0;
};
} // namespace common
#endif
