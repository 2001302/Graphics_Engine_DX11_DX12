#ifndef _ENTITY
#define _ENTITY

#include <string>

namespace common {
class IEntity {
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
