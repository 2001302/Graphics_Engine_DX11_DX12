#include "Direct3D.h"

using namespace Engine;

bool Direct3D::Init(Env* env, bool vsync, HWND hwnd, bool fullscreen)
{
	InitDirect3D(env->screenWidth, env->screenHeight, hwnd, fullscreen);
	InitMainScene(env->screenWidth, env->screenHeight);

	return true;
}

bool Direct3D::InitDirect3D(int screenWidth, int screenHeight, HWND hwnd, bool fullscreen)
{
	HRESULT result;
	unsigned int numModes, i, numerator, denominator;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;

	// Initialize the swap chain description.
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set to a single back buffer.
	swapChainDesc.BufferCount = 1;

	// Set the width and height of the back buffer.
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;

	// Set regular 32-bit surface for the back buffer.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Set the usage of the back buffer.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// Set the handle for the window to render to.
	swapChainDesc.OutputWindow = hwnd;

	// Turn multisampling off.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// Set to full screen or windowed mode.
	if (fullscreen)
		swapChainDesc.Windowed = false;
	else
		swapChainDesc.Windowed = true;

	// Set the scan line ordering and scaling to unspecified.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Discard the back buffer contents after presenting.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// Don't set the advanced flags.
	swapChainDesc.Flags = 0;

	// Set the feature level to DirectX 11.
	featureLevel = D3D_FEATURE_LEVEL_11_0;

	// Create the swap chain, Direct3D device, and Direct3D device context.
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext);
	if (FAILED(result))
	{
		return false;
	}
}

bool Direct3D::InitMainScene(int screenWidth, int screenHeight)
{

	float fieldOfView, screenAspect;

	{
		// Get the pointer to the back buffer.
		ComPtr<ID3D11Texture2D> backBufferPtr;
		m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)backBufferPtr.GetAddressOf());
		// Create the render target view with the back buffer pointer.
		m_device->CreateRenderTargetView(backBufferPtr.Get(), NULL, &renderTargetView);
		// Release pointer to the back buffer as we no longer need it.
		backBufferPtr->Release();
	}

	{
		//Initialize the description of the depth buffer.
		D3D11_TEXTURE2D_DESC depthBufferDesc;
		ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

		//Set up the description of the depth buffer.
		depthBufferDesc.Width = screenWidth;
		depthBufferDesc.Height = screenHeight;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.ArraySize = 1;
		depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthBufferDesc.CPUAccessFlags = 0;
		depthBufferDesc.MiscFlags = 0;
		//Create the texture for the depth buffer using the filled out description.
		m_device->CreateTexture2D(&depthBufferDesc, NULL, depthStencilBuffer.GetAddressOf());

	}

	{
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
		// Initialize the description of the stencil state.
		ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

		// Set up the description of the stencil state.
		depthStencilDesc.DepthEnable = true;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

		depthStencilDesc.StencilEnable = true;
		depthStencilDesc.StencilReadMask = 0xFF;
		depthStencilDesc.StencilWriteMask = 0xFF;

		// Stencil operations if pixel is front-facing.
		depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Stencil operations if pixel is back-facing.
		depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Create the depth stencil state.
		m_device->CreateDepthStencilState(&depthStencilDesc, depthStencilState.GetAddressOf());
	}

	{
		// Set the depth stencil state.
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		m_deviceContext->OMSetDepthStencilState(depthStencilState.Get(), 1);

		// Initialize the depth stencil view.
		ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

		// Set up the depth stencil view description.
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		// Create the depth stencil view.
		m_device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, &depthStencilView);

	}

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	m_deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView.Get());

	{
		D3D11_RASTERIZER_DESC rasterDesc;

		// Setup the raster description which will determine how and what polygons will be drawn.
		rasterDesc.AntialiasedLineEnable = false;
		rasterDesc.CullMode = D3D11_CULL_BACK;
		rasterDesc.DepthBias = 0;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = true;
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.FrontCounterClockwise = false;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.ScissorEnable = false;
		rasterDesc.SlopeScaledDepthBias = 0.0f;

		// Create the rasterizer state from the description we just filled out.
		m_device->CreateRasterizerState(&rasterDesc, solidRasterizerSate.GetAddressOf());
		// Now set the rasterizer state.
		m_deviceContext->RSSetState(solidRasterizerSate.Get());
	}

	{
		// Create a rasterizer state
		D3D11_RASTERIZER_DESC rastDescWire;
		ZeroMemory(&rastDescWire, sizeof(D3D11_RASTERIZER_DESC)); // Need this
		rastDescWire.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
		// rastDescWire.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
		rastDescWire.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
		rastDescWire.FrontCounterClockwise = false;
		rastDescWire.DepthClipEnable = true; // <- zNear, zFar 확인에 필요

		m_device->CreateRasterizerState(&rastDescWire,
			wireRasterizerSate.GetAddressOf());

	}

	{
		SetViewPort(0.0f, 0.0f, (float)screenWidth, (float)screenHeight);
	}

	return true;
}

void Direct3D::SetViewPort(float x, float y, float width, float height)
{
	const float SCREEN_DEPTH = 1000.0f;
	const float SCREEN_NEAR = 0.3f;

	// Setup the viewport for rendering.
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = x;
	viewport.TopLeftY = y;

	// Create the viewport.
	m_deviceContext->RSSetViewports(1, &viewport);
}

void Direct3D::Shutdown()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if (m_swapChain)
	{
		m_swapChain->SetFullscreenState(false, NULL);
	}

	if (m_deviceContext)
	{
		m_deviceContext->Release();
	}

	if (m_device)
	{
		m_device->Release();
	}

	if (m_swapChain)
	{
		m_swapChain->Release();
	}

	if (solidRasterizerSate)
	{
		solidRasterizerSate->Release();
	}

	if (wireRasterizerSate)
	{
		wireRasterizerSate->Release();
	}

	if (depthStencilView)
	{
		depthStencilView->Release();
	}

	if (depthStencilState)
	{
		depthStencilState->Release();
	}

	if (depthStencilBuffer)
	{
		depthStencilBuffer->Release();
	}

	if (renderTargetView)
	{
		renderTargetView->Release();
	}

	return;
}

void Direct3D::BeginScene(float red, float green, float blue, float alpha)
{
	float color[4];

	// Setup the color to clear the buffer to.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// Clear the back buffer.
	m_deviceContext->ClearRenderTargetView(renderTargetView, color);

	// Clear the depth buffer.
	m_deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;
}

void Direct3D::EndScene()
{
	// Present the back buffer to the screen since rendering is complete.
	if (m_vsync_enabled)
	{
		// Lock to screen refresh rate.
		m_swapChain->Present(1, 0);
	}
	else
	{
		// Present as fast as possible.
		m_swapChain->Present(0, 0);
	}

	return;
}

ComPtr<ID3D11Device> Direct3D::GetDevice()
{
	return m_device;
}

ComPtr<ID3D11DeviceContext> Direct3D::GetDeviceContext()
{
	return m_deviceContext;
}
