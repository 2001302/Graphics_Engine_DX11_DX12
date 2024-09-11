#ifndef _GRAPHICS_CORE
#define _GRAPHICS_CORE

#include "back_buffer.h"
#include "color_buffer.h"
#include "depth_buffer.h"
#include "command_manager.h"
#include "heap_manager.h"

namespace graphics {
struct GlobalGpuBuffer {
    BackBuffer back_buffer;
    ColorBuffer hdr_buffer;
    ColorBuffer ldr_buffer;
    DepthBuffer dsv_buffer;
};

class GpuCore {
  public:
    static GpuCore &Instance() {
        static GpuCore instance;
        return instance;
    }
    bool Initialize();
    bool InitializePix();
    bool InitializeDevice() {
        UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
#endif

        ComPtr<IDXGIFactory4> factory;
        ASSERT_FAILED(
            CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

        bool useWarpDevice = true;
        if (useWarpDevice) {
            ComPtr<IDXGIAdapter> warpAdapter;
            ASSERT_FAILED(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
            ASSERT_FAILED(D3D12CreateDevice(warpAdapter.Get(),
                                            D3D_FEATURE_LEVEL_11_0,
                                            IID_PPV_ARGS(&device)));
        } else {
            ComPtr<IDXGIAdapter1> hardwareAdapter;
            // GetHardwareAdapter(factory.Get(), &hardwareAdapter);
            ASSERT_FAILED(D3D12CreateDevice(hardwareAdapter.Get(),
                                            D3D_FEATURE_LEVEL_11_0,
                                            IID_PPV_ARGS(&device)));
        }

        // Describe and create the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
        swapChainDesc.Width = foundation::Env::Instance().screen_width;
        swapChainDesc.Height = foundation::Env::Instance().screen_height;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;

        ASSERT_FAILED(factory->CreateSwapChainForHwnd(
            device.Get(), foundation::Env::Instance().main_window,
            &swapChainDesc, nullptr, nullptr, swap_chain.GetAddressOf()));

        ASSERT_FAILED(factory->MakeWindowAssociation(
            foundation::Env::Instance().main_window, DXGI_MWA_NO_ALT_ENTER));
    };

    bool InitializeBuffer() {
        buffer_manager.back_buffer.Create(
            device.Get(), &heap_manager->GetRTVHeap(), swap_chain.Get());
        buffer_manager.hdr_buffer.Create(
            device.Get(), &heap_manager->GetRTVHeap(),
            &heap_manager->GetViewHeap(), DXGI_FORMAT_R16G16B16A16_FLOAT);
        buffer_manager.ldr_buffer.Create(
            device.Get(), &heap_manager->GetRTVHeap(),
            &heap_manager->GetViewHeap(), DXGI_FORMAT_R8G8B8A8_UNORM);
        buffer_manager.dsv_buffer.Create(
            device.Get(), &heap_manager->GetDSVHeap(), DXGI_FORMAT_D32_FLOAT);

        buffer_manager.back_buffer.Allocate();
        buffer_manager.hdr_buffer.Allocate();
        buffer_manager.ldr_buffer.Allocate();
        buffer_manager.dsv_buffer.Allocate();
        return true;
    };
    bool InitializeCommand() {
        command_manager = std::make_shared<GpuCommandManager>();
        command_manager->Initialize(device.Get());
        return true;
    };
    bool InitializeHeap() {
        heap_manager = std::make_shared<GpuHeapManager>();
        heap_manager->Initialize(device.Get(), 1024);
        return true;
    };

    GlobalGpuBuffer Buffer() { return buffer_manager; }

    ID3D12Device *GetDevice() { return device.Get(); }
    IDXGISwapChain1 *GetSwapChain() { return swap_chain.Get(); }
    GpuHeapManager *GetHeapMgr() { return heap_manager.get(); }
    GpuCommandManager *GetCommandMgr() { return command_manager.get(); }

  private:
    GpuCore() : swap_chain(0), device(0), heap_manager(0), command_manager(0) {}
    ComPtr<IDXGISwapChain1> swap_chain;
    ComPtr<ID3D12Device> device;
    std::shared_ptr<GpuHeapManager> heap_manager;
    std::shared_ptr<GpuCommandManager> command_manager;
    GlobalGpuBuffer buffer_manager;
};

} // namespace graphics
#endif
