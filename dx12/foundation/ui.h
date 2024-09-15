#ifndef _BASEGUI
#define _BASEGUI

#include "dataBlock.h"
#include "node.h"

namespace common {
class IGui : public IDataBlock {
  public:
    bool Start();
    bool Frame();
    void Shutdown();

    virtual void OnStart(){};
    virtual void OnFrame(){};

    ImVec2 GetSize() { return gui_size; }

  protected:
    ImGuiContext *context_ = nullptr;
    std::string ini_file_name_;
    ImVec2 gui_size;

  private:
    void RecreateFontAtlas();
};
} // namespace common
#endif
