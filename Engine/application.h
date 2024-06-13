#ifndef _APPLICATION
#define _APPLICATION

#include "platform.h"

namespace Engine {
class Application : public Platform {
  public:
    Application() : imgui_(0), manager_(0), env_(0){};

    bool OnStart() override final;
    bool OnFrame() override final;
    bool OnStop() override final;
    LRESULT CALLBACK MessageHandler(HWND main_window, UINT umsg, WPARAM wparam,
                                    LPARAM lparam) override final;

  private:
    bool OnModelLoadRequest();
    bool OnSphereLoadRequest();
    bool OnBoxLoadRequest();
    bool OnCylinderLoadRequest();

    bool OnRightDragRequest();
    bool OnMouseWheelRequest();

    std::unique_ptr<Input> input_;
    ImGuiManager *imgui_;
    PipelineManager *manager_;
    Env *env_;
};
} // namespace Engine
#endif