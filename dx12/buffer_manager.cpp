#include "buffer_manager.h"

namespace graphics {

bool GpuBuffer::Initialize() {

    InitializeBuffer();
    AllocateBuffer();
    return true;
}

bool GpuBuffer::InitializeBuffer() {

    back_buffer.Create(GpuCore::Instance().GetDevice(),
                       GpuCore::Instance().GetHeap().RTV(),
                       GpuCore::Instance().GetSwapChain());
    hdr_buffer.Create(
        GpuCore::Instance().GetDevice(), GpuCore::Instance().GetHeap().RTV(),
        GpuCore::Instance().GetHeap().View(), DXGI_FORMAT_R16G16B16A16_FLOAT);
    dsv_buffer.Create(GpuCore::Instance().GetDevice(),
                      GpuCore::Instance().GetHeap().DSV(),
                      DXGI_FORMAT_D32_FLOAT);

    return true;
};

bool GpuBuffer::AllocateBuffer() {
    back_buffer.Allocate();
    hdr_buffer.Allocate();
    dsv_buffer.Allocate();
    return true;
};

void GpuBuffer::Shutdown(){
    // TODO: release all resources
};
} // namespace graphics
