#ifndef _APPLICATION
#define _APPLICATION

#include "platform.h"

namespace Engine {
class Application : public Platform{
  public:
    Application()
        : m_imgui(0), m_manager(0), m_env(0){ };
    ~Application();

    bool OnStart() override final;
    bool OnFrame() override final;
    bool OnStop()override final;
    LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM) override final;

  private:
    bool OnModelLoadRequest();
    bool OnSphereLoadRequest();
    bool OnBoxLoadRequest();
    bool OnCylinderLoadRequest();

    bool OnRightDragRequest();
    bool OnMouseWheelRequest();

    std::unique_ptr<Input> m_input;
    ImGuiManager *m_imgui;
    PipelineManager *m_manager;
    Env *m_env;
};
} // namespace Engine
#endif