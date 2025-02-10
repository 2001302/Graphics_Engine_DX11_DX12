#ifndef _NODEUI
#define _NODEUI

#include "../gui/ImGui/imgui.h"
#include "../gui/ImGui/imgui_impl_dx12.h"
#include "../gui/ImGui/imgui_impl_dx11.h"
#include "../gui/ImGui/imgui_impl_win32.h"
#include "../gui/ImGui/imgui_internal.h"
#include "../gui/ImGui/NodeEditor/imgui_node_editor.h"
#include "entity.h"

namespace ed = ax::NodeEditor;

namespace common {

struct INode : public IEntity {
  public:
    struct LinkInfo {
        ed::LinkId Id;
        ed::PinId InputId;
        ed::PinId OutputId;
    };

    void Show();

    int uniqueId = 1;
    ImVec2 position = ImVec2(0, 0);

  private:
    virtual void OnShow() {}

  protected:
    ImVector<LinkInfo> links_;
    int next_link_Id = 100;

    bool firstframe = true;
};
} // namespace common
#endif
