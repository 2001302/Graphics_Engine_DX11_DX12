#ifndef _BUFFER_MANAGER
#define _BUFFER_MANAGER

#include "../buffer/back_buffer.h"
#include "../buffer/color_buffer.h"
#include "../buffer/depth_buffer.h"
#include "device_manager.h"
#include "../buffer/texture_2d.h"
#include "../buffer/texture_cube.h"
#include <structure/env.h>

namespace graphics {
class GpuBuffer {
  public:
    static GpuBuffer &Instance() {
        static GpuBuffer instance;
        return instance;
    }
    bool Initialize();
    bool InitializeBuffer();
    void Shutdown();

    BackBuffer *GetDisplay() { return back_buffer; }
    ColorBuffer *GetHDR() { return hdr_buffer; }
    DepthBuffer *GetDSV() { return dsv_buffer; }

  private:
    GpuBuffer() : back_buffer(0), hdr_buffer(0), dsv_buffer(0){};

    BackBuffer *back_buffer;
    ColorBuffer *hdr_buffer;
    DepthBuffer *dsv_buffer;
};

} // namespace graphics
#endif
