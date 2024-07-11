#ifndef _BASEGUI
#define _BASEGUI

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_node_editor.h"
#include "ImGui/imgui_internal.h"
#include "dataBlock.h"
#include "direct3D.h"

namespace ed = ax::NodeEditor;

namespace Engine {
class IGui : public IDataBlock {
  public:
    bool Initialize();
    bool Frame();
    void Shutdown();

    virtual void OnStart(){};
    virtual void OnFrame(float delta_time){};

  protected:
    ImGuiContext *context_ = nullptr;
    std::string ini_file_name_;
    ImFont *default_font_ = nullptr;
    ImFont *header_font = nullptr;

  private:
    void RecreateFontAtlas();
};
} // namespace Engine
#endif
