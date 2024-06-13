#ifndef _APPLICATION
#define _APPLICATION

#include "platform.h"
#include "message_receiver.h"

namespace Engine {
class Application : public Platform {
  public:
    Application()
        : imgui_(0), manager_(0), env_(0) {
        message_receiver_ = std::make_unique<MessageReceiver>();
    };

    bool OnStart() override final;
    bool OnFrame() override final;
    bool OnStop() override final;
    LRESULT CALLBACK MessageHandler(HWND main_window, UINT umsg, WPARAM wparam,
                                    LPARAM lparam) override final;

  private:
    std::shared_ptr<Input> input_;
    ImGuiManager *imgui_;
    PipelineManager *manager_;
    Env *env_;
    std::unique_ptr<MessageReceiver> message_receiver_;
};
} // namespace Engine
#endif