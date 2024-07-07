#include "direct3D.h"

using namespace Engine;

bool Direct3D::Initialize(Env *env, bool vsync, HWND main_window,
                          bool fullscreen) {
    const D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;

    UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;

    const D3D_FEATURE_LEVEL featureLevels[2] = {
        D3D_FEATURE_LEVEL_11_0, // 더 높은 버전이 먼저 오도록 설정
        D3D_FEATURE_LEVEL_9_3};
    D3D_FEATURE_LEVEL featureLevel;

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferDesc.Width = env->screen_width_;
    sd.BufferDesc.Height = env->screen_height_;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferCount = 2;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = main_window;
    sd.Windowed = TRUE;
    sd.Flags =
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // allow full-screen switching
    // sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //ImGui 폰트가 두꺼워짐
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.SampleDesc.Count = 1; // _FLIP_은 MSAA 미지원
    sd.SampleDesc.Quality = 0;

    ThrowIfFailed(D3D11CreateDeviceAndSwapChain(
        0, driverType, 0, createDeviceFlags, featureLevels, 1,
        D3D11_SDK_VERSION, &sd, swap_chain_.GetAddressOf(),
        device_.GetAddressOf(), &featureLevel, device_context_.GetAddressOf()));

    if (featureLevel != D3D_FEATURE_LEVEL_11_0) {
        std::cout << "D3D Feature Level 11 unsupported." << std::endl;
        return false;
    }

    CreateBuffer(env);

    SetViewPort(0.0f, 0.0f, (float)env->screen_width_,
                (float)env->screen_height_);

    D3D11_RASTERIZER_DESC rastDesc;
    ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));
    rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
    rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
    rastDesc.FrontCounterClockwise = false;
    rastDesc.DepthClipEnable = true;

    ThrowIfFailed(device_->CreateRasterizerState(
        &rastDesc, solid_rasterizer_state_.GetAddressOf()));

    rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;

    ThrowIfFailed(device_->CreateRasterizerState(
        &rastDesc, wire_rasterizer_state_.GetAddressOf()));

    return true;
}

bool Direct3D::CreateBuffer(Env *env) {

    // 레스터화 -> float/depthBuffer(MSAA) -> resolved -> backBuffer

    // BackBuffer는 화면으로 최종 출력되기 때문에  RTV만 필요하고 SRV는 불필요
    ComPtr<ID3D11Texture2D> backBuffer;
    ThrowIfFailed(
        swap_chain_->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));
    ThrowIfFailed(device_->CreateRenderTargetView(
        backBuffer.Get(), NULL, back_buffer_RTV.GetAddressOf()));

    // FLOAT MSAA RenderTargetView/ShaderResourceView
    ThrowIfFailed(device_->CheckMultisampleQualityLevels(
        DXGI_FORMAT_R16G16B16A16_FLOAT, 4, &num_quality_levels_));

    D3D11_TEXTURE2D_DESC desc;
    backBuffer->GetDesc(&desc);
    desc.MipLevels = desc.ArraySize = 1;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    desc.Usage = D3D11_USAGE_DEFAULT; // 스테이징 텍스춰로부터 복사 가능
    desc.MiscFlags = 0;
    desc.CPUAccessFlags = 0;
    if (useMSAA && num_quality_levels_) {
        desc.SampleDesc.Count = 4;
        desc.SampleDesc.Quality = num_quality_levels_ - 1;
    } else {
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
    }

    ThrowIfFailed(
        device_->CreateTexture2D(&desc, NULL, float_buffer.GetAddressOf()));

    ThrowIfFailed(device_->CreateShaderResourceView(float_buffer.Get(), NULL,
                                                    float_SRV.GetAddressOf()));

    ThrowIfFailed(device_->CreateRenderTargetView(float_buffer.Get(), NULL,
                                                  float_RTV.GetAddressOf()));

    CreateDepthBuffer(device_, env->screen_width_, env->screen_height_,
                      UINT(useMSAA ? num_quality_levels_ : 0),
                      depth_stencil_view_);

    // FLOAT MSAA를 Relsolve해서 저장할 SRV/RTV
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    ThrowIfFailed(
        device_->CreateTexture2D(&desc, NULL, resolved_buffer.GetAddressOf()));
    ThrowIfFailed(device_->CreateShaderResourceView(
        resolved_buffer.Get(), NULL, resolved_SRV.GetAddressOf()));
    ThrowIfFailed(device_->CreateRenderTargetView(resolved_buffer.Get(), NULL,
                                                  resolved_RTV.GetAddressOf()));

    // Todo
    // m_postProcess.Initialize(m_device, m_context, {m_resolvedSRV},
    //                          {m_backBufferRTV}, m_screenWidth,
    //                          m_screenHeight, 4);
    return true;
}

void Direct3D::CreateDepthBuffer(
    ComPtr<ID3D11Device> &device, int screenWidth, int screenHeight,
    UINT numQualityLevels, ComPtr<ID3D11DepthStencilView> &depthStencilView) {

    D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
    depthStencilBufferDesc.Width = screenWidth;
    depthStencilBufferDesc.Height = screenHeight;
    depthStencilBufferDesc.MipLevels = 1;
    depthStencilBufferDesc.ArraySize = 1;
    depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    if (numQualityLevels > 0) {
        depthStencilBufferDesc.SampleDesc.Count = 4; // how many multisamples
        depthStencilBufferDesc.SampleDesc.Quality = numQualityLevels - 1;
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
        depthStencilBuffer.Get(), 0, depthStencilView.GetAddressOf()));
}

void Direct3D::SetViewPort(float x, float y, float width, float height) {
    const float SCREEN_DEPTH = 1000.0f;
    const float SCREEN_NEAR = 0.3f;

    // Setup the viewport for rendering.
    viewport_.Width = (float)width;
    viewport_.Height = (float)height;
    viewport_.MinDepth = 0.0f;
    viewport_.MaxDepth = 1.0f;
    viewport_.TopLeftX = x;
    viewport_.TopLeftY = y;

    // Create the viewport.
    device_context_->RSSetViewports(1, &viewport_);
}

void Direct3D::BeginScene(float red, float green, float blue, float alpha) {
    float color[4];

    // Setup the color to clear the buffer to.
    color[0] = red;
    color[1] = green;
    color[2] = blue;
    color[3] = alpha;

    std::vector<ID3D11RenderTargetView *> renderTargetViews = {float_RTV.Get()};
    // Clear the back buffer.
    for (size_t i = 0; i < renderTargetViews.size(); i++) {
        device_context_->ClearRenderTargetView(renderTargetViews[i], color);
    }
    device_context_->OMSetRenderTargets(UINT(renderTargetViews.size()),
                                        renderTargetViews.data(),
                                        depth_stencil_view_.Get());

    // Clear the depth buffer.
    device_context_->ClearDepthStencilView(depth_stencil_view_.Get(),
                                           D3D11_CLEAR_DEPTH, 1.0f, 0);
    device_context_->OMSetDepthStencilState(depth_stencil_state_.Get(), 0);

    return;
}

void Direct3D::EndScene() {
    
    // Present the back buffer to the screen since rendering is complete.
    if (vsync_enabled_) {
        // Lock to screen refresh rate.
        swap_chain_->Present(1, 0);
    } else {
        // Present as fast as possible.
        swap_chain_->Present(0, 0);
    }

    return;
}

ComPtr<ID3D11Device> Direct3D::device() { return device_; }

ComPtr<ID3D11DeviceContext> Direct3D::device_context() {
    return device_context_;
}

ComPtr<IDXGISwapChain> Direct3D::swap_chain() { return swap_chain_; }

ComPtr<ID3D11Texture2D> Direct3D::depth_stencil_buffer() {
    return float_buffer;
};

ComPtr<ID3D11DepthStencilState> Direct3D::depth_stencil_state() {
    return depth_stencil_state_;
}

ComPtr<ID3D11DepthStencilView> Direct3D::depth_stencil_view() {
    return depth_stencil_view_;
}

ComPtr<ID3D11RasterizerState> Direct3D::solid_rasterizer_state() {
    return solid_rasterizer_state_;
}

ComPtr<ID3D11RasterizerState> Direct3D::wire_rasterizer_state() {
    return wire_rasterizer_state_;
}

ComPtr<ID3D11RenderTargetView> Direct3D::render_target_view() {
    return float_RTV;
}

D3D11_VIEWPORT Direct3D::viewport() { return viewport_; }