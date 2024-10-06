#include "buffer_manager.h"

namespace graphics {

bool GpuBuffer::Initialize() {

    InitializeBuffer();
    return true;
}

bool GpuBuffer::InitializeBuffer() {
    back_buffer = BackBuffer::Create(GpuCore::Instance().GetDevice(),
                                     GpuCore::Instance().GetHeap().RTV(),
                                     GpuCore::Instance().GetSwapChain());
    hdr_buffer = ColorBuffer::Create(common::env::screen_width,
                                     common::env::screen_height,
                                     DXGI_FORMAT_R16G16B16A16_FLOAT);
    dsv_buffer = DepthBuffer::Create(common::env::screen_width,
                                     common::env::screen_height,
                                     DXGI_FORMAT_D24_UNORM_S8_UINT);
    return true;
};

void GpuBuffer::Shutdown() {
    if (back_buffer) {
        delete back_buffer;
        back_buffer = 0;
    }
    if (hdr_buffer) {
        delete hdr_buffer;
        hdr_buffer = 0;
    }
    if (dsv_buffer) {
        delete dsv_buffer;
        dsv_buffer = 0;
    }
};
} // namespace graphics
