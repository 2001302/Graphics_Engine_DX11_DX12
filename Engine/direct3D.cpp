#include "direct3D.h"

using namespace Engine;

bool Direct3D::Initialize(Env *env, bool vsync, HWND hwnd, bool fullscreen) {
    HRESULT result;
    DXGI_SWAP_CHAIN_DESC swap_chain_desc;
    D3D_FEATURE_LEVEL feature_level;

    // Initialize the swap chain description.
    ZeroMemory(&swap_chain_desc, sizeof(swap_chain_desc));

    // Set to a single back buffer.
    swap_chain_desc.BufferCount = 1;

    // Set the width and height of the back buffer.
    swap_chain_desc.BufferDesc.Width = env->screen_width_;
    swap_chain_desc.BufferDesc.Height = env->screen_height_;

    // Set regular 32-bit surface for the back buffer.
    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    // Set the usage of the back buffer.
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    // Set the handle for the window to render to.
    swap_chain_desc.OutputWindow = hwnd;

    // Turn multisampling off.
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.SampleDesc.Quality = 0;

    // Set to full screen or windowed mode.
    if (fullscreen)
        swap_chain_desc.Windowed = false;
    else
        swap_chain_desc.Windowed = true;

    // Set the scan line ordering and scaling to unspecified.
    swap_chain_desc.BufferDesc.ScanlineOrdering =
        DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    // Discard the back buffer contents after presenting.
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    // Don't set the advanced flags.
    swap_chain_desc.Flags = 0;

    // Set the feature level to DirectX 11.
    feature_level = D3D_FEATURE_LEVEL_11_0;

    // Create the swap chain, Direct3D device, and Direct3D device context.
    result = D3D11CreateDeviceAndSwapChain(
        NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &feature_level, 1,
        D3D11_SDK_VERSION, &swap_chain_desc, &swap_chain_, &device_, NULL,
        &device_context_);
    if (FAILED(result)) {
        return false;
    }

    CreateRenderTargetView();

    CreateDepthBuffer(env);

    {
        D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
        // Initialize the description of the stencil state.
        ZeroMemory(&depth_stencil_desc, sizeof(depth_stencil_desc));

        // Set up the description of the stencil state.
        depth_stencil_desc.DepthEnable = true;
        depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;

        depth_stencil_desc.StencilEnable = true;
        depth_stencil_desc.StencilReadMask = 0xFF;
        depth_stencil_desc.StencilWriteMask = 0xFF;

        // Stencil operations if pixel is front-facing.
        depth_stencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
        depth_stencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        depth_stencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        // Stencil operations if pixel is back-facing.
        depth_stencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        depth_stencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
        depth_stencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        depth_stencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        // Create the depth stencil state.
        device_->CreateDepthStencilState(&depth_stencil_desc,
                                         depth_stencil_state_.GetAddressOf());
    }

    {
        // Set the depth stencil state.
        D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc;
        device_context_->OMSetDepthStencilState(depth_stencil_state_.Get(), 1);

        // Initialize the depth stencil view.
        ZeroMemory(&depth_stencil_view_desc, sizeof(depth_stencil_view_desc));

        // Set up the depth stencil view description.
        depth_stencil_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        depth_stencil_view_desc.Texture2D.MipSlice = 0;

        // Create the depth stencil view.
        device_->CreateDepthStencilView(depth_stencil_buffer_.Get(),
                                        &depth_stencil_view_desc,
                                        &depth_stencil_view_);
    }

    // Bind the render target view and depth stencil buffer to the output render
    // pipeline.
    device_context_->OMSetRenderTargets(1, &render_target_view_,
                                        depth_stencil_view_.Get());

    {
        D3D11_RASTERIZER_DESC raster_desc;

        // Setup the raster description which will determine how and what
        // polygons will be drawn.
        raster_desc.AntialiasedLineEnable = false;
        raster_desc.CullMode = D3D11_CULL_BACK;
        raster_desc.DepthBias = 0;
        raster_desc.DepthBiasClamp = 0.0f;
        raster_desc.DepthClipEnable = true;
        raster_desc.FillMode = D3D11_FILL_SOLID;
        raster_desc.FrontCounterClockwise = false;
        raster_desc.MultisampleEnable = false;
        raster_desc.ScissorEnable = false;
        raster_desc.SlopeScaledDepthBias = 0.0f;

        // Create the rasterizer state from the description we just filled out.
        device_->CreateRasterizerState(&raster_desc,
                                       solid_rasterizer_state_.GetAddressOf());
        // Now set the rasterizer state.
        device_context_->RSSetState(solid_rasterizer_state_.Get());
    }

    {
        // Create a rasterizer state
        D3D11_RASTERIZER_DESC rast_desc_wire;
        ZeroMemory(&rast_desc_wire, sizeof(D3D11_RASTERIZER_DESC)); // Need this
        rast_desc_wire.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
        // rast_desc_wire.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
        rast_desc_wire.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
        rast_desc_wire.FrontCounterClockwise = false;
        rast_desc_wire.DepthClipEnable = true; // <- zNear, zFar 확인에 필요

        device_->CreateRasterizerState(&rast_desc_wire,
                                       wire_rasterizer_state_.GetAddressOf());
    }

    {
        SetViewPort(0.0f, 0.0f, (float)env->screen_width_,
                    (float)env->screen_height_);
    }

    return true;
}

bool Direct3D::CreateRenderTargetView() {
    
        // Get the pointer to the back buffer.
        ComPtr<ID3D11Texture2D> backBufferPtr;
        swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D),
                               (LPVOID *)backBufferPtr.GetAddressOf());
        // Create the render target view with the back buffer pointer.
        device_->CreateRenderTargetView(backBufferPtr.Get(), NULL,
                                        &render_target_view_);
        // Release pointer to the back buffer as we no longer need it.
        backBufferPtr->Release();

        return true;
}

bool Direct3D::CreateDepthBuffer(Env *env) {
    
        // Initialize the description of the depth buffer.
        D3D11_TEXTURE2D_DESC depth_buffer_desc;
        ZeroMemory(&depth_buffer_desc, sizeof(depth_buffer_desc));

        // Set up the description of the depth buffer.
        depth_buffer_desc.Width = env->screen_width_;
        depth_buffer_desc.Height = env->screen_height_;
        depth_buffer_desc.MipLevels = 1;
        depth_buffer_desc.ArraySize = 1;
        depth_buffer_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depth_buffer_desc.SampleDesc.Count = 1;
        depth_buffer_desc.SampleDesc.Quality = 0;
        depth_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        depth_buffer_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depth_buffer_desc.CPUAccessFlags = 0;
        depth_buffer_desc.MiscFlags = 0;
        // Create the texture for the depth buffer using the filled out
        // description.
        device_->CreateTexture2D(&depth_buffer_desc, NULL,
                                 depth_stencil_buffer_.GetAddressOf());
        return true;
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

    // Clear the back buffer.
    device_context_->ClearRenderTargetView(render_target_view_, color);

    // Clear the depth buffer.
    device_context_->ClearDepthStencilView(depth_stencil_view_.Get(),
                                           D3D11_CLEAR_DEPTH, 1.0f, 0);

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
    return depth_stencil_buffer_;
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

ID3D11RenderTargetView **Direct3D::render_target_view() {
    return &render_target_view_;
}

D3D11_VIEWPORT Direct3D::viewport() { return viewport_; }