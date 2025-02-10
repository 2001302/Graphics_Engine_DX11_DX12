#include "device_manager.h"

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

    ComPtr<IDXGIAdapter1> hardware_adapter;
    ASSERT_FAILED(D3D12CreateDevice(
        hardware_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));
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
        command_manager.Queue(D3D12_COMMAND_LIST_TYPE_DIRECT)->Get(),
        common::env::main_window, &swapChainDesc, nullptr, nullptr,
        swap_chain.GetAddressOf()));

    ASSERT_FAILED(factory->MakeWindowAssociation(common::env::main_window,
                                                 DXGI_MWA_NO_ALT_ENTER));
};

bool GpuCore::InitializeCommand() {
    query_heap.Initialize(device.Get());
    command_manager.Initialize(device.Get(), query_heap.Get());
    return true;
};

bool GpuCore::InitializeHeap() {
    heap_manager.Initialize(device.Get(), 1024);
    return true;
};

void GpuCore::Shutdown(){
    // TODO: release all resources
};
} // namespace graphics
