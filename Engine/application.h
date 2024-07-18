#ifndef _APPLICATION
#define _APPLICATION

#include "behavior_tree.h"
#include "direct3D.h"
#include "geometry_generator.h"
#include "message.h"
#include "message_receiver.h"
#include "pipeline_manager.h"
#include "platform.h"
#include "setting_ui.h"

namespace platform {
class Application : public Platform {
  public:
    Application();

    bool OnStart() override final;
    bool OnFrame() override final;
    bool OnStop() override final;
    LRESULT CALLBACK MessageHandler(HWND main_window, UINT umsg, WPARAM wparam,
                                    LPARAM lparam) override final;

  private:
    std::shared_ptr<dx11::Input> input_;
    std::shared_ptr<common::SettingUi> imgui_;
    std::shared_ptr<dx11::PipelineManager> manager_;
    std::unique_ptr<dx11::MessageReceiver> message_receiver_;
};
} // namespace platform
#endif
