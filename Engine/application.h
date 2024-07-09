#ifndef _APPLICATION
#define _APPLICATION

#include "message_receiver.h"
#include "panel.h"
#include "platform.h"

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
    std::shared_ptr<Panel> imgui_;
    std::shared_ptr<PipelineManager> manager_;
    std::unique_ptr<MessageReceiver> message_receiver_;
};
} // namespace Engine
#endif
