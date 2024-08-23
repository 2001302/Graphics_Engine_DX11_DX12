#include "graphics_core.h"

#include <filesystem>
#include <knownfolders.h>
#include <shlobj.h>

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

namespace dx12 {

bool GpuCore::Initialize() {
    if (GetModuleHandle(L"WinPixGpuCapturer.dll") == 0) {
        LoadLibrary(GetLatestWinPixGpuCapturerPath_Cpp17().c_str());
    }

    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the
    // active device.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    bool useWarpDevice = true;
    if (useWarpDevice) {
        ComPtr<IDXGIAdapter> warpAdapter;
        ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
        ThrowIfFailed(D3D12CreateDevice(
            warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));
    } else {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        // GetHardwareAdapter(factory.Get(), &hardwareAdapter);
        ThrowIfFailed(D3D12CreateDevice(hardwareAdapter.Get(),
                                        D3D_FEATURE_LEVEL_11_0,
                                        IID_PPV_ARGS(&device)));
    }

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(
        device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&command_queue)));

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Width = foundation::Env::Instance().screen_width;
    swapChainDesc.Height = foundation::Env::Instance().screen_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(factory->CreateSwapChainForHwnd(
        command_queue.Get(), // Swap chain needs the queue so that it can force
                             // a flush on it.
        foundation::Env::Instance().main_window, &swapChainDesc, nullptr,
        nullptr, &swapChain));

    // This sample does not support fullscreen transitions.
    ThrowIfFailed(factory->MakeWindowAssociation(
        foundation::Env::Instance().main_window, DXGI_MWA_NO_ALT_ENTER));

    ThrowIfFailed(swapChain.As(&swap_chain));
    frame_index = swap_chain->GetCurrentBackBufferIndex();

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = 2;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(
            device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtv_heap)));

        rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(
            D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
            rtv_heap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < 2; n++) {
            ThrowIfFailed(
                swap_chain->GetBuffer(n, IID_PPV_ARGS(&render_targets[n])));
            device->CreateRenderTargetView(render_targets[n].Get(), nullptr,
                                           rtvHandle);
            rtvHandle.Offset(1, rtv_descriptor_size);
        }
    }

    ThrowIfFailed(
        device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
    fence_value = 1;

    // Create an event handle to use for frame synchronization.
    fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (fence_event == nullptr) {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }

    return true;
}
} // namespace dx12
