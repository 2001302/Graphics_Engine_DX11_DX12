#ifndef _IDATABLOCK
#define _IDATABLOCK

namespace common {
/// <summary>
/// Abstract class
/// BehaviorTree traversal 수행 시 사용되는 DataBlock 단위.
/// </summary>
class IDataBlock {
  public:
    virtual ~IDataBlock(){};
};

} // namespace common
#endif
