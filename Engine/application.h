#ifndef _APPLICATION
#define _APPLICATION

#include "behavior_tree.h"
#include "direct3D.h"
#include "geometry_generator.h"
#include "message.h"
#include "message_receiver.h"
#include "pipeline_manager.h"
#include "platform.h"
#include "resource_helper.h"
#include "setting_ui.h"

namespace dx11 {
class Application : public Platform {
  public:
    Application();

    bool OnStart() override final;
    bool OnFrame() override final;
    bool OnStop() override final;
    LRESULT CALLBACK MessageHandler(HWND main_window, UINT umsg, WPARAM wparam,
                                    LPARAM lparam) override final;

  private:
    std::shared_ptr<Input> input_;
    std::shared_ptr<common::SettingUi> imgui_;
    std::shared_ptr<PipelineManager> manager_;
    std::unique_ptr<MessageReceiver> message_receiver_;
};
} // namespace platform
#endif
