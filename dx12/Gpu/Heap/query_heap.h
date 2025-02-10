#ifndef _QUERY_HEAP
#define _QUERY_HEAP

#include "../pso/pipeline_state_object.h"
#include "../../pch.h"
#include <d3d12.h>
#include <wrl/client.h>

namespace graphics {
class QueryHeap {
  public:
    QueryHeap(){};
    void Initialize(ID3D12Device *device) {
        D3D12_QUERY_HEAP_DESC query_heap_desc = {
            D3D12_QUERY_HEAP_TYPE_TIMESTAMP, 2, 0};
        ASSERT_FAILED(device->CreateQueryHeap(&query_heap_desc,
                                              IID_PPV_ARGS(&query_heap)));
    };
    ID3D12QueryHeap *Get() { return query_heap.Get(); }

  private:
    ComPtr<ID3D12QueryHeap> query_heap;
};

} // namespace graphics
#endif