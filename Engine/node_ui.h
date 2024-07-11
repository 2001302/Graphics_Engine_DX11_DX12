#ifndef _NODEUI
#define _NODEUI

#include "gui_base.h"
#include "graph.h"

namespace Engine {

struct INodeUi {
  public:
    void Show() { OnShow(); }
  private:
    virtual void OnShow() {}
  protected:
    ImVector<LinkInfo> links_;
    int next_link_Id = 100;
};
} // namespace Engine
#endif
