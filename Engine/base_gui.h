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
class BaseGui : public IDataBlock {
  public:
    bool Initialize(HWND main_window, Env *env);
    bool Frame();
    void Shutdown();

    virtual void OnStart(){};
    virtual void OnFrame(float delta_time){};

  protected:
    HWND main_window_;
    Env *env_;
    ImGuiContext *m_Context = nullptr;
    std::string m_IniFilename;
    ImFont *m_DefaultFont = nullptr;
    ImFont *m_HeaderFont = nullptr;

  private:
    void RecreateFontAtlas();
};
} // namespace Engine
#endif