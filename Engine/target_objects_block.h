#ifndef _TARGETOBJECTSBLOCK
#define _TARGETOBJECTSBLOCK

#include <vector>
#include "dataBlock.h"

namespace Engine {

class TargetObjectsBlock : public IDataBlock {
  public:
    std::vector<int> target_ids;
};
} // namespace Engine
#endif
