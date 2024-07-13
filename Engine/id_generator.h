#ifndef _IDGENERATOR
#define _IDGENERATOR

namespace common {
class IdGenerator {
  public:
    IdGenerator(){};
    ~IdGenerator(){};

    static int GetId();
    static void Reset();

  private:
    static inline int entity_id_ = 10000;
};

} // namespace Engine
#endif
