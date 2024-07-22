#ifndef _APPLICATION
#define _APPLICATION

#include "behavior_tree.h"
#include "geometry_generator.h"
#include "graphics_manager.h"
#include "input.h"
#include "message.h"
#include "message_receiver.h"
#include "pipeline_manager.h"
#include "platform.h"
#include "setting_ui.h"

namespace engine {
class Application : public Platform {
  public:
    Application();

    bool OnStart() override final;
    bool OnFrame() override final;
    bool OnStop() override final;
    LRESULT CALLBACK MessageHandler(HWND main_window, UINT umsg, WPARAM wparam,
                                    LPARAM lparam) override final;

  private:
    std::shared_ptr<PipelineManager> manager_;
    std::unique_ptr<MessageReceiver> message_receiver_;
    std::shared_ptr<Input> input_;
    std::shared_ptr<common::SettingUi> imgui_;
};
} // namespace engine
#endif
