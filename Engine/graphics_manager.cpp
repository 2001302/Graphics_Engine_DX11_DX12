#include "graphics_manager.h"

namespace engine {

bool GraphicsManager::Initialize() {
    const D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;

    UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    const D3D_FEATURE_LEVEL featureLevels[2] = {D3D_FEATURE_LEVEL_11_0,
                                                D3D_FEATURE_LEVEL_9_3};
    D3D_FEATURE_LEVEL featureLevel;

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferDesc.Width = common::Env::Instance().screen_width;
    sd.BufferDesc.Height = common::Env::Instance().screen_height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferCount = 2;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = common::Env::Instance().main_window;
    sd.Windowed = TRUE;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.SampleDesc.Count = 1; // _FLIP_은 MSAA 미지원
    sd.SampleDesc.Quality = 0;

    ThrowIfFailed(D3D11CreateDeviceAndSwapChain(
        0, driverType, 0, createDeviceFlags, featureLevels, 1,
        D3D11_SDK_VERSION, &sd, swap_chain.GetAddressOf(),
        device.GetAddressOf(), &featureLevel, device_context.GetAddressOf()));

    if (featureLevel != D3D_FEATURE_LEVEL_11_0) {
        std::cout << "D3D Feature Level 11 unsupported." << std::endl;
        return false;
    }

    CreateBuffer();

    SetViewPort(0.0f, 0.0f, (float)common::Env::Instance().screen_width,
                (float)common::Env::Instance().screen_height);

    D3D11_RASTERIZER_DESC rastDesc;
    ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));
    rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
    rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
    rastDesc.FrontCounterClockwise = false;
    rastDesc.DepthClipEnable = true;

    ThrowIfFailed(device->CreateRasterizerState(
        &rastDesc, solid_rasterizer_state.GetAddressOf()));

    rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;

    ThrowIfFailed(device->CreateRasterizerState(
        &rastDesc, wire_rasterizer_state.GetAddressOf()));

    //Graphics::InitCommonStates(device);

    return true;
}

bool GraphicsManager::CreateBuffer() {

    // 레스터화 -> float/depthBuffer(MSAA) -> resolved -> backBuffer

    // BackBuffer는 화면으로 최종 출력되기 때문에  RTV만 필요하고 SRV는 불필요
    ComPtr<ID3D11Texture2D> backBuffer;
    ThrowIfFailed(
        swap_chain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));
    ThrowIfFailed(device->CreateRenderTargetView(
        backBuffer.Get(), NULL, back_buffer_RTV.GetAddressOf()));

    // FLOAT MSAA RenderTargetView/ShaderResourceView
    ThrowIfFailed(device->CheckMultisampleQualityLevels(
        DXGI_FORMAT_R16G16B16A16_FLOAT, 4, &num_quality_levels));

    D3D11_TEXTURE2D_DESC desc;
    backBuffer->GetDesc(&desc);
    desc.MipLevels = desc.ArraySize = 1;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    desc.Usage = D3D11_USAGE_DEFAULT; // 스테이징 텍스춰로부터 복사 가능
    desc.MiscFlags = 0;
    desc.CPUAccessFlags = 0;
    if (useMSAA && num_quality_levels) {
        desc.SampleDesc.Count = 4;
        desc.SampleDesc.Quality = num_quality_levels - 1;
    } else {
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
    }

    ThrowIfFailed(
        device->CreateTexture2D(&desc, NULL, float_buffer.GetAddressOf()));

    ThrowIfFailed(device->CreateShaderResourceView(float_buffer.Get(), NULL,
                                                   float_SRV.GetAddressOf()));

    ThrowIfFailed(device->CreateRenderTargetView(float_buffer.Get(), NULL,
                                                 float_RTV.GetAddressOf()));

    CreateDepthBuffer();

    // FLOAT MSAA를 Relsolve해서 저장할 SRV/RTV
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    ThrowIfFailed(
        device->CreateTexture2D(&desc, NULL, resolved_buffer.GetAddressOf()));
    ThrowIfFailed(device->CreateShaderResourceView(
        resolved_buffer.Get(), NULL, resolved_SRV.GetAddressOf()));
    ThrowIfFailed(device->CreateRenderTargetView(resolved_buffer.Get(), NULL,
                                                 resolved_RTV.GetAddressOf()));

    return true;
}

void GraphicsManager::CreateDepthBuffer() {

    D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
    depthStencilBufferDesc.Width = common::Env::Instance().screen_width;
    depthStencilBufferDesc.Height = common::Env::Instance().screen_height;
    depthStencilBufferDesc.MipLevels = 1;
    depthStencilBufferDesc.ArraySize = 1;
    depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    if (num_quality_levels > 0) {
        depthStencilBufferDesc.SampleDesc.Count = 4; // how many multisamples
        depthStencilBufferDesc.SampleDesc.Quality = num_quality_levels - 1;
    } else {
        depthStencilBufferDesc.SampleDesc.Count = 1; // how many multisamples
        depthStencilBufferDesc.SampleDesc.Quality = 0;
    }
    depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilBufferDesc.CPUAccessFlags = 0;
    depthStencilBufferDesc.MiscFlags = 0;

    ComPtr<ID3D11Texture2D> depthStencilBuffer;

    ThrowIfFailed(device->CreateTexture2D(&depthStencilBufferDesc, 0,
                                          depthStencilBuffer.GetAddressOf()));

    ThrowIfFailed(device->CreateDepthStencilView(
        depthStencilBuffer.Get(), 0, depth_stencil_view.GetAddressOf()));
}

void GraphicsManager::SetViewPort(float x, float y, float width, float height) {
    // Setup the viewport for rendering.
    viewport.Width = (float)width;
    viewport.Height = (float)height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = x;
    viewport.TopLeftY = y;

    // Create the viewport.
    device_context->RSSetViewports(1, &viewport);
}

void GraphicsManager::BeginScene(bool draw_as_wire) {
    float color[4];

    // Setup the color to clear the buffer to.
    color[0] = 0.0f;
    color[1] = 0.0f;
    color[2] = 0.0f;
    color[3] = 1.0f;

    std::vector<ID3D11RenderTargetView *> rtv = {float_RTV.Get()};
    // Clear the back buffer.
    for (size_t i = 0; i < rtv.size(); i++) {
        device_context->ClearRenderTargetView(rtv[i], color);
    }
    device_context->OMSetRenderTargets(UINT(rtv.size()), rtv.data(),
                                       depth_stencil_view.Get());

    // Clear the depth buffer.
    device_context->ClearDepthStencilView(depth_stencil_view.Get(),
                                          D3D11_CLEAR_DEPTH, 1.0f, 0);
    device_context->OMSetDepthStencilState(depth_stencil_state.Get(), 0);

    device_context->OMSetRenderTargets(1, float_RTV.GetAddressOf(),
                                       depth_stencil_view.Get());
    device_context->OMSetDepthStencilState(depth_stencil_state.Get(), 0);

    if (draw_as_wire)
        device_context->RSSetState(wire_rasterizer_state.Get());
    else
        device_context->RSSetState(solid_rasterizer_state.Get());

    return;
}

void GraphicsManager::EndScene() {

    if (common::Env::Instance().vsync_enabled) {
        swap_chain->Present(1, 0);
    } else {
        swap_chain->Present(0, 0);
    }

    return;
}
} // namespace dx11