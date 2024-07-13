#ifndef _ENTITY
#define _ENTITY

#include "node_ui.h"
#include <string>

namespace common {
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
} // namespace common
#endif
