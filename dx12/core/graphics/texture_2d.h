#ifndef _TEXTURE2D
#define _TEXTURE2D

#include "gpu_buffer.h"
#include <filesystem>

namespace dx12 {
class Texture2D : public GpuBuffer {
  public:
    Texture2D() : GpuBuffer(){};
    Texture2D(int width, int height, DXGI_FORMAT format);
    void Allocate(GpuHeap *heap, UINT &index) override;
};
} // namespace dx12
#endif
