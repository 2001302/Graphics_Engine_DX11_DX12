#include "graphics_manager.h"

namespace core {

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
    SetMainViewport();

    D3D11_RASTERIZER_DESC rastDesc;
    ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));
    rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
    rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
    rastDesc.FrontCounterClockwise = false;
    rastDesc.DepthClipEnable = true;

    graphics::InitCommonStates(device);

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
    ThrowIfFailed(device->CreateRenderTargetView(float_buffer.Get(), NULL,
                                                 float_RTV.GetAddressOf()));

    // FLOAT MSAA를 Relsolve해서 저장할 SRV/RTV
    GraphicsUtil::CreateUATexture(
        device, common::Env::Instance().screen_width,
        common::Env::Instance().screen_height, DXGI_FORMAT_R16G16B16A16_FLOAT,
        resolved_buffer, resolved_RTV, resolved_SRV, resolved_UAV);

    CreateDepthBuffer();

    return true;
}

void GraphicsManager::CreateDepthBuffer() {

    D3D11_TEXTURE2D_DESC desc;
    desc.Width = common::Env::Instance().screen_width;
    desc.Height = common::Env::Instance().screen_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    if (num_quality_levels > 0) {
        desc.SampleDesc.Count = 4; // how many multisamples
        desc.SampleDesc.Quality = num_quality_levels - 1;
    } else {
        desc.SampleDesc.Count = 1; // how many multisamples
        desc.SampleDesc.Quality = 0;
    }
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    ComPtr<ID3D11Texture2D> depthStencilBuffer;

    ThrowIfFailed(
        device->CreateTexture2D(&desc, 0, depthStencilBuffer.GetAddressOf()));

    ThrowIfFailed(device->CreateDepthStencilView(
        depthStencilBuffer.Get(), 0, m_depthStencilView.GetAddressOf()));
}

void GraphicsManager::SetMainViewport() {

    // Set the viewport
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = float(common::Env::Instance().screen_width);
    viewport.Height = float(common::Env::Instance().screen_height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    device_context->RSSetViewports(1, &viewport);
}

void GraphicsManager::SetPipelineState(const GraphicsPSO &pso) {

    device_context->VSSetShader(pso.vertex_shader.Get(), 0, 0);
    device_context->PSSetShader(pso.pixel_shader.Get(), 0, 0);
    device_context->HSSetShader(pso.hull_shader.Get(), 0, 0);
    device_context->DSSetShader(pso.domain_shader.Get(), 0, 0);
    device_context->GSSetShader(pso.geometry_shader.Get(), 0, 0);
    device_context->IASetInputLayout(pso.input_layout.Get());
    device_context->RSSetState(pso.rasterizer_state.Get());
    device_context->OMSetBlendState(pso.blend_state.Get(), pso.blend_factor,
                                    0xffffffff);
    device_context->OMSetDepthStencilState(pso.depth_stencil_state.Get(),
                                           pso.stencil_ref);
    device_context->IASetPrimitiveTopology(pso.primitive_topology);
}

void GraphicsManager::SetPipelineState(const ComputePSO &pso) {
    device_context->VSSetShader(NULL, 0, 0);
    device_context->PSSetShader(NULL, 0, 0);
    device_context->HSSetShader(NULL, 0, 0);
    device_context->DSSetShader(NULL, 0, 0);
    device_context->GSSetShader(NULL, 0, 0);
    device_context->CSSetShader(pso.compute_shader.Get(), 0, 0);
}

void GraphicsManager::SetGlobalConsts(ComPtr<ID3D11Buffer> &globalConstsGPU) {
    // 쉐이더와 일관성 유지 cbuffer GlobalConstants : register(b0)
    device_context->VSSetConstantBuffers(0, 1, globalConstsGPU.GetAddressOf());
    device_context->PSSetConstantBuffers(0, 1, globalConstsGPU.GetAddressOf());
    device_context->GSSetConstantBuffers(0, 1, globalConstsGPU.GetAddressOf());
}

} // namespace engine