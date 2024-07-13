#ifndef _NODEUI
#define _NODEUI

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_node_editor.h"
#include "ImGui/imgui_internal.h"

namespace ed = ax::NodeEditor;

namespace common {

struct INodeUi {
  public:
    struct LinkInfo {
        ed::LinkId Id;
        ed::PinId InputId;
        ed::PinId OutputId;
    };

    void Show();

  private:
    virtual void OnShow() {}

  protected:
    ImVector<LinkInfo> links_;
    int next_link_Id = 100;
};
} // namespace Engine
#endif
