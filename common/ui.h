#ifndef _BASEGUI
#define _BASEGUI

#include "dataBlock.h"
#include "node.h"

namespace common {
class IGui : public IDataBlock {
  public:
    bool Start();
    bool Frame();

    virtual void OnStart(){};
    virtual void OnFrame(){};

  protected:
    ImGuiContext *context_ = nullptr;
    std::string ini_file_name_;

  private:
    void RecreateFontAtlas();
};
} // namespace common
#endif
