#ifndef _BASEGUI
#define _BASEGUI

#include "dataBlock.h"
#include "graphics_context.h"
#include "node_ui.h"

namespace common {
class IGui : public IDataBlock {
  public:
    bool Initialize();
    bool FrameBegin();
    bool FrameEnd();
    void Shutdown();

    virtual void OnStart(){};
    virtual void OnFrame(){};

  protected:
    ImGuiContext *context_ = nullptr;
    std::string ini_file_name_;
    ImFont *default_font_ = nullptr;
    ImFont *header_font = nullptr;

  private:
    void RecreateFontAtlas();
};
} // namespace common
#endif
