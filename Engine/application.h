#ifndef _APPLICATION
#define _APPLICATION

#include "platform.h"
#include "direct3D.h"
#include "setting_ui.h"
#include "behavior_tree.h"
#include "pipeline_manager.h"

#include "message_receiver.h"
#include "geometry_generator.h"
#include "resource_helper.h"

namespace Engine {
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
    std::shared_ptr<SettingUi> imgui_;
    std::shared_ptr<PipelineManager> manager_;
    std::unique_ptr<MessageReceiver> message_receiver_;
};
} // namespace Engine
#endif
