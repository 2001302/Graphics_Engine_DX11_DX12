#ifndef _APPLICATION
#define _APPLICATION

#include "env.h"
#include "direct3D.h"
#include "imgui_manager.h"
#include "behavior_tree.h"
#include "behavior_leaf.h"
#include "pipeline_manager.h"
#include "resource_helper.h"
#include "geometry_generator.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.3f;

namespace Engine {
class Application {
  public:
    Application()
        : m_imgui(0), m_manager(0), m_env(0), m_mainWindow(0), m_hinstance(0){};
    Application(HWND mainWindow, HINSTANCE hinstance)
        : m_imgui(0), m_manager(0), m_env(0) {
        m_mainWindow = mainWindow;
        m_hinstance = hinstance;
    };

    PipelineManager *GetManager() { return m_manager; }
    bool Initialize(int screenWidth, int screenHeight);
    void Shutdown();
    bool Frame();
    bool Render();

    bool OnModelLoadRequest();
    bool OnSphereLoadRequest();
    bool OnBoxLoadRequest();
    bool OnCylinderLoadRequest();

    bool OnRightDragRequest();
    bool OnMouseWheelRequest();

  private:
    std::unique_ptr<Input> m_input;
    ImGuiManager *m_imgui;
    PipelineManager *m_manager;
    Env *m_env;
    HWND m_mainWindow;
    HINSTANCE m_hinstance;
};
} // namespace Engine
#endif