#ifndef _COMMAND_LIST
#define _COMMAND_LIST

#include "descriptor_heap.h"
#include <d3d12.h>
#include <memory>
#include <vector>
#include <wrl/client.h>

namespace graphics {
class CommandList {
  public:
    CommandList() : command_allocator_(0), command_list_(0){};

    ID3D12CommandAllocator *GetAllocator() { return command_allocator_; };
    ID3D12GraphicsCommandList *GetList() { return command_list_; }
    
  private:
    ID3D12CommandAllocator* command_allocator_;
    ID3D12GraphicsCommandList* command_list_;
};
} // namespace graphics
#endif
