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

namespace graphics {

bool GpuCore::Initialize() {
#if defined(_DEBUG)
    if (GetModuleHandle(L"WinPixGpuCapturer.dll") == 0) {
        LoadLibrary(GetLatestWinPixGpuCapturerPath_Cpp17().c_str());
    }
#endif

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

    ThrowIfFailed(factory->CreateSwapChainForHwnd(
        device.Get(), foundation::Env::Instance().main_window, &swapChainDesc,
        nullptr, nullptr, swap_chain.GetAddressOf()));

    ThrowIfFailed(factory->MakeWindowAssociation(
        foundation::Env::Instance().main_window, DXGI_MWA_NO_ALT_ENTER));

    // Heap
    heap_manager = new GpuHeapManager();

    heap_manager->rtv_heap =
        new DescriptorHeap(1024, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1);
    heap_manager->view_heap =
        new DescriptorHeap(1024, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
    heap_manager->dsv_heap =
        new DescriptorHeap(1024, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
    heap_manager->sampler_heap =
        new DescriptorHeap(1024, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 1);

    // Buffer
    buffer_manager = new GpuBufferManager();

    buffer_manager->back_buffer = new BackBuffer(swap_chain.Get());
    buffer_manager->back_buffer->Allocate(device.Get(), heap_manager->rtv_heap);

    buffer_manager->hdr_buffer =
        new ColorBuffer(device.Get(), DXGI_FORMAT_R16G16B16A16_FLOAT);
    buffer_manager->hdr_buffer->Allocate(device.Get(), heap_manager->rtv_heap,
                                         heap_manager->view_heap);

    buffer_manager->ldr_buffer =
        new ColorBuffer(device.Get(), DXGI_FORMAT_R8G8B8A8_UNORM);
    buffer_manager->ldr_buffer->Allocate(device.Get(), heap_manager->rtv_heap,
                                         heap_manager->view_heap);

    buffer_manager->dsv_buffer =
        new DepthBuffer(device.Get(), DXGI_FORMAT_R16G16B16A16_FLOAT);
    buffer_manager->dsv_buffer->Allocate(device.Get(), heap_manager->dsv_heap);

    // Command
    command_manager = new GpuCommandManager(device.Get());

    context_manager = new GpuContextManager();

    command_manager->CreateNewCommandList(
        device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT,
        &context_manager->graphics_list,
        &context_manager->graphics_list_allocator);
    command_manager->CreateNewCommandList(
        device.Get(), D3D12_COMMAND_LIST_TYPE_COMPUTE,
        &context_manager->compute_list,
        &context_manager->compute_list_allocator);
    command_manager->CreateNewCommandList(
        device.Get(), D3D12_COMMAND_LIST_TYPE_COPY, &context_manager->copy_list,
        &context_manager->copy_list_allocator);

    return true;
}
} // namespace graphics
