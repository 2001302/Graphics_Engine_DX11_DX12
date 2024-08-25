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

bool GpuCore::InitializeGPU() {
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
        ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc,
                                                   IID_PPV_ARGS(&heap_FLIP)));

        UINT desc_size_RTV = device->GetDescriptorHandleIncrementSize(
            D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Create frame resources.
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
            heap_FLIP->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < 2; n++) {
            ThrowIfFailed(
                swap_chain->GetBuffer(n, IID_PPV_ARGS(&resource_FLIP[n])));
            device->CreateRenderTargetView(resource_FLIP[n].Get(), nullptr,
                                           rtvHandle);
            rtvHandle.Offset(1, desc_size_RTV);
        }
    }

    {
        scissorRect = CD3DX12_RECT(
            0, 0, static_cast<LONG>(foundation::Env::Instance().screen_width),
            static_cast<LONG>(foundation::Env::Instance().screen_height));
    }

    {
        memset(&viewport, 0, sizeof(D3D12_VIEWPORT));
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = (float)foundation::Env::Instance().screen_width;
        viewport.Height = (float)foundation::Env::Instance().screen_height;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
    }

    CreateBuffer();

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
void GpuCore::CreateBuffer() {
    if (useMSAA) {
        DXGI_SAMPLE_DESC sampleDesc = {};
        sampleDesc.Count = 4;
        sampleDesc.Quality = 0;

        D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
        msQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        msQualityLevels.SampleCount = sampleDesc.Count;
        msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
        msQualityLevels.NumQualityLevels = 0;

        ThrowIfFailed(device->CheckFeatureSupport(
            D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msQualityLevels,
            sizeof(msQualityLevels)));

        if (msQualityLevels.NumQualityLevels > 0) {
            sampleDesc.Quality = msQualityLevels.NumQualityLevels - 1;
        } else {
            sampleDesc.Count = 1;
            sampleDesc.Quality = 0;
        }

        D3D12_RESOURCE_DESC resource_desc_RTV = {};
        resource_desc_RTV.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resource_desc_RTV.Alignment = 0;
        resource_desc_RTV.Width = foundation::Env::Instance().screen_width;
        resource_desc_RTV.Height = foundation::Env::Instance().screen_height;
        resource_desc_RTV.DepthOrArraySize = 1;
        resource_desc_RTV.MipLevels = 1;
        resource_desc_RTV.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        resource_desc_RTV.SampleDesc = sampleDesc;
        resource_desc_RTV.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resource_desc_RTV.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        const float clear_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        D3D12_CLEAR_VALUE clear_value_RTV = {};
        clear_value_RTV.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        memcpy(clear_value_RTV.Color, clear_color, sizeof(clear_color));

        auto heap_property = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        ThrowIfFailed(device->CreateCommittedResource(
            &heap_property, D3D12_HEAP_FLAG_NONE, &resource_desc_RTV,
            D3D12_RESOURCE_STATE_RENDER_TARGET, &clear_value_RTV,
            IID_PPV_ARGS(&resource_MS)));

        // rtv
        D3D12_DESCRIPTOR_HEAP_DESC heap_desc_RTV = {};
        heap_desc_RTV.NumDescriptors = 1;
        heap_desc_RTV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        heap_desc_RTV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(device->CreateDescriptorHeap(&heap_desc_RTV,
                                                   IID_PPV_ARGS(&heap_MS)));

        D3D12_RENDER_TARGET_VIEW_DESC desc_RTV = {};
        desc_RTV.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        desc_RTV.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
        device->CreateRenderTargetView(
            resource_MS.Get(), &desc_RTV,
            heap_MS->GetCPUDescriptorHandleForHeapStart());

        // dsv
        D3D12_CLEAR_VALUE clear_value_DSV = {};
        clear_value_DSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        clear_value_DSV.DepthStencil.Depth = 1.0f;
        clear_value_DSV.DepthStencil.Stencil = 0;

        D3D12_RESOURCE_DESC resource_desc_DSV = {};
        resource_desc_DSV.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resource_desc_DSV.Alignment = 0;
        resource_desc_DSV.Width = foundation::Env::Instance().screen_width;
        resource_desc_DSV.Height = foundation::Env::Instance().screen_height;
        resource_desc_DSV.DepthOrArraySize = 1;
        resource_desc_DSV.MipLevels = 1;
        resource_desc_DSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        resource_desc_DSV.SampleDesc = sampleDesc;
        resource_desc_DSV.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resource_desc_DSV.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        ThrowIfFailed(device->CreateCommittedResource(
            &heap_property, D3D12_HEAP_FLAG_NONE, &resource_desc_DSV,
            D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear_value_DSV,
            IID_PPV_ARGS(&resourcce_DSV)));

        D3D12_DESCRIPTOR_HEAP_DESC heap_desc_DSV = {};
        heap_desc_DSV.NumDescriptors = 1;
        heap_desc_DSV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        heap_desc_DSV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(device->CreateDescriptorHeap(&heap_desc_DSV,
                                                   IID_PPV_ARGS(&heap_DSV)));

        D3D12_DEPTH_STENCIL_VIEW_DESC desc_DSV = {};
        desc_DSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        desc_DSV.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
        desc_DSV.Flags = D3D12_DSV_FLAG_NONE;

        device->CreateDepthStencilView(
            resourcce_DSV.Get(), &desc_DSV,
            heap_DSV->GetCPUDescriptorHandleForHeapStart());
    }
    {
        // resolved buffer
        D3D12_RESOURCE_DESC resource_desc_RTV = {};
        resource_desc_RTV.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resource_desc_RTV.Alignment = 0;
        resource_desc_RTV.Width = foundation::Env::Instance().screen_width;
        resource_desc_RTV.Height = foundation::Env::Instance().screen_height;
        resource_desc_RTV.DepthOrArraySize = 1;
        resource_desc_RTV.MipLevels = 1;
        resource_desc_RTV.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        resource_desc_RTV.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resource_desc_RTV.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        resource_desc_RTV.SampleDesc.Count = 1;
        resource_desc_RTV.SampleDesc.Quality = 0;

        const float clear_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        D3D12_CLEAR_VALUE clear_value_RTV = {};
        clear_value_RTV.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        memcpy(clear_value_RTV.Color, clear_color, sizeof(clear_color));

        auto heap_property = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        ThrowIfFailed(device->CreateCommittedResource(
            &heap_property, D3D12_HEAP_FLAG_NONE, &resource_desc_RTV,
            D3D12_RESOURCE_STATE_COMMON, &clear_value_RTV,
            IID_PPV_ARGS(&resource_resolved)));

        D3D12_DESCRIPTOR_HEAP_DESC heap_desc_RTV = {};
        heap_desc_RTV.NumDescriptors = 1;
        heap_desc_RTV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heap_desc_RTV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(device->CreateDescriptorHeap(
            &heap_desc_RTV, IID_PPV_ARGS(&heap_resolved)));

        //D3D12_RENDER_TARGET_VIEW_DESC desc_RTV = {};
        //desc_RTV.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        //desc_RTV.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        //device->CreateRenderTargetView(
        //    resource_resolved.Get(), &desc_RTV,
        //    heap_resolved->GetCPUDescriptorHandleForHeapStart());
    }

    {
        // staging buffer
        D3D12_RESOURCE_DESC resource_desc_RTV = {};
        resource_desc_RTV.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resource_desc_RTV.Alignment = 0;
        resource_desc_RTV.Width = foundation::Env::Instance().screen_width;
        resource_desc_RTV.Height = foundation::Env::Instance().screen_height;
        resource_desc_RTV.DepthOrArraySize = 1;
        resource_desc_RTV.MipLevels = 1;
        resource_desc_RTV.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        resource_desc_RTV.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resource_desc_RTV.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        resource_desc_RTV.SampleDesc.Count = 1;
        resource_desc_RTV.SampleDesc.Quality = 0;

        auto heap_property = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        ThrowIfFailed(device->CreateCommittedResource(
            &heap_property, D3D12_HEAP_FLAG_NONE, &resource_desc_RTV,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr,
            IID_PPV_ARGS(&resource_staging)));

        D3D12_DESCRIPTOR_HEAP_DESC heap_desc_RTV = {};
        heap_desc_RTV.NumDescriptors = 1;
        heap_desc_RTV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heap_desc_RTV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(device->CreateDescriptorHeap(
            &heap_desc_RTV, IID_PPV_ARGS(&heap_staging)));
    }
}
CD3DX12_CPU_DESCRIPTOR_HANDLE GpuCore::GetHandleFloatRTV() {
    CD3DX12_CPU_DESCRIPTOR_HANDLE
    rtvHandle(heap_MS->GetCPUDescriptorHandleForHeapStart());
    return rtvHandle;
};
CD3DX12_CPU_DESCRIPTOR_HANDLE GpuCore::GetHandleResolvedRTV() {
    CD3DX12_CPU_DESCRIPTOR_HANDLE
    rtvHandle(heap_resolved->GetCPUDescriptorHandleForHeapStart());
    return rtvHandle;
};
CD3DX12_CPU_DESCRIPTOR_HANDLE GpuCore::GetHandleRTV() {
    UINT desc_size_RTV = device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        heap_FLIP->GetCPUDescriptorHandleForHeapStart(), frame_index,
        desc_size_RTV);
    return rtvHandle;
};
CD3DX12_CPU_DESCRIPTOR_HANDLE GpuCore::GetHandleDSV() {
    CD3DX12_CPU_DESCRIPTOR_HANDLE
    dsvHandle(heap_DSV->GetCPUDescriptorHandleForHeapStart());
    return dsvHandle;
};
} // namespace dx12
