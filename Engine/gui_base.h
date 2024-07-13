#ifndef _BASEGUI
#define _BASEGUI

#include "dataBlock.h"
#include "direct3D.h"
#include "node_ui.h"

namespace Engine {
class IGui : public IDataBlock {
  public:
    bool Initialize();
    bool Frame(std::unordered_map<int, std::shared_ptr<INodeUi>> node_map);
    void Shutdown();

    virtual void OnStart(){};
    virtual void OnFrame(){};

  protected:
    ImGuiContext *context_ = nullptr;
    std::string ini_file_name_;
    ImFont *default_font_ = nullptr;
    ImFont *header_font = nullptr;
    std::unordered_map<int, std::shared_ptr<INodeUi>> node_map;

  private:
    void RecreateFontAtlas();
};
} // namespace Engine
#endif
