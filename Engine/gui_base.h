#ifndef _BASEGUI
#define _BASEGUI

#include "dataBlock.h"
#include "graphics_manager.h"
#include "node_ui.h"

namespace common {
class IGui : public IDataBlock {
  public:
    bool Initialize();
    bool Frame(IDataBlock *dataBlock = nullptr);
    void Shutdown();

    virtual void OnStart(){};
    virtual void OnFrame(IDataBlock *dataBlock){};

    ImVec2 GetSize() { return gui_size; }
  protected:
    ImGuiContext *context_ = nullptr;
    std::string ini_file_name_;
    ImFont *default_font_ = nullptr;
    ImFont *header_font = nullptr;
    ImVec2 gui_size;

  private:
    void RecreateFontAtlas();
};
} // namespace common
#endif
