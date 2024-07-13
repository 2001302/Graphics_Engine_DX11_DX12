#ifndef _ENTITY
#define _ENTITY

#include "common_struct.h"
#include "node_ui.h"

namespace Engine {
using Microsoft::WRL::ComPtr;

class IEntity : public INodeUi {
  public:
    IEntity();
    ~IEntity();
    int GetEntityId() const;
    std::string GetName() const;
    void SetName(std::string name);

  private:
    int entity_id_;
    std::string name_ = "";
};
} // namespace Engine
#endif
