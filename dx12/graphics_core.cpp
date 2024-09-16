#include "graphics_core.h"

#include <filesystem>
#include <knownfolders.h>
#include <shlobj.h>

namespace graphics {

bool GpuCore::Initialize() {

    InitializePix();
    InitializeDevice();
    InitializeCommand();
    InitializeSwapchain();
    InitializeHeap();
    InitializeBuffer();
    return true;
}

static std::wstring GetLatestWinPixGpuCapturerPath_Cpp17() {
    LPWSTR programFilesPath = nullptr;
    SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, NULL,
                         &programFilesPath);

    std::filesystem::path pixInstallationPath = programFilesPath;
    pixInstallationPath /= "Microsoft PIX";

    std::wstring newestVersionFound;

    for (auto const &directory_entry :
         std::filesystem::directory_iterator(pixInstallationPath)) {
        if (directory_entry.is_directory()) {
            if (newestVersionFound.empty() ||
                newestVersionFound <
                    directory_entry.path().filename().c_str()) {
                newestVersionFound = directory_entry.path().filename().c_str();
            }
        }
    }

    if (newestVersionFound.empty()) {
        // TODO: Error, no PIX installation found
    }

    return pixInstallationPath / newestVersionFound / L"WinPixGpuCapturer.dll";
}

bool GpuCore::InitializePix() {
    if (GetModuleHandle(L"WinPixGpuCapturer.dll") == 0) {
        LoadLibrary(GetLatestWinPixGpuCapturerPath_Cpp17().c_str());
    }
    return true;
}

bool GpuCore::InitializeDevice() {
    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();

        // Enable additional debug layers.
        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif
    Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
    ASSERT_FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    bool useWarpDevice = true;
    if (useWarpDevice) {
        ComPtr<IDXGIAdapter> warpAdapter;
        ASSERT_FAILED(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
        ASSERT_FAILED(D3D12CreateDevice(
            warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));
    } else {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        // GetHardwareAdapter(factory.Get(), &hardwareAdapter);
        ASSERT_FAILED(D3D12CreateDevice(hardwareAdapter.Get(),
                                        D3D_FEATURE_LEVEL_11_0,
                                        IID_PPV_ARGS(&device)));
    }
};

bool GpuCore::InitializeSwapchain() {
    Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
    ASSERT_FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)));

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
    swapChainDesc.Width = common::env::screen_width;
    swapChainDesc.Height = common::env::screen_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;

    ASSERT_FAILED(factory->CreateSwapChainForHwnd(
        command_manager.Queue()->Get(), common::env::main_window,
        &swapChainDesc, nullptr, nullptr, swap_chain.GetAddressOf()));

    ASSERT_FAILED(factory->MakeWindowAssociation(common::env::main_window,
                                                 DXGI_MWA_NO_ALT_ENTER));
};

bool GpuCore::InitializeBuffer() {

    buffers.hdr_buffer = new ColorBuffer();
    buffers.ldr_buffer = new ColorBuffer();
    buffers.dsv_buffer = new DepthBuffer();

    buffers.hdr_buffer->Create(device.Get(), heap_manager.RTV(),
                               heap_manager.View(),
                               DXGI_FORMAT_R16G16B16A16_FLOAT);
    buffers.ldr_buffer->Create(device.Get(), heap_manager.RTV(),
                               heap_manager.View(),
                               DXGI_FORMAT_R16G16B16A16_FLOAT);
    buffers.dsv_buffer->Create(device.Get(), heap_manager.DSV(),
                               DXGI_FORMAT_D32_FLOAT);

    buffers.hdr_buffer->Allocate();
    buffers.ldr_buffer->Allocate();
    buffers.dsv_buffer->Allocate();
    return true;
};

bool GpuCore::InitializeCommand() {
    command_manager.Initialize(device.Get());
    return true;
};

bool GpuCore::InitializeHeap() {
    heap_manager.Initialize(device.Get(), 1024);

    back_buffer.Create(device.Get(), heap_manager.RTV(), swap_chain.Get());
    back_buffer.Allocate();
    return true;
};

void GpuCore::Shutdown(){
    // TODO: release all resources
};
} // namespace graphics
