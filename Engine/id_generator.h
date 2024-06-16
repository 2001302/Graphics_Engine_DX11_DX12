#ifndef _IDGENERATOR
#define _IDGENERATOR

namespace Engine {
class IdGenerator {
  public:
    IdGenerator(){};
    ~IdGenerator(){};

    static int GetId();
    static void Reset();

  private:
    static inline int entity_id_ = 0;
};

} // namespace Engine
#endif
