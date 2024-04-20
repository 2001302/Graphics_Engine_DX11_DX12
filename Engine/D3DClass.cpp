#include "d3dclass.h"

using namespace Engine;

bool D3DClass::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear)
{
	InitializeGraphicDevice(screenWidth, screenHeight, hwnd, fullscreen);
	InitializeMainScene(screenWidth, screenHeight, screenDepth, screenNear);

    return true;
}

bool D3DClass::InitializeGraphicDevice(int screenWidth, int screenHeight, HWND hwnd, bool fullscreen)
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
bool D3DClass::InitializeMainScene(int screenWidth, int screenHeight,float screenDepth, float screenNear) 
{
	HRESULT result;

	float fieldOfView, screenAspect;

	auto view = ViewInfo();

	{
		// Get the pointer to the back buffer.
		ID3D11Texture2D* backBufferPtr;
		result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
		if (FAILED(result))
		{
			return false;
		}

		// Create the render target view with the back buffer pointer.
		result = m_device->CreateRenderTargetView(backBufferPtr, NULL, &view.RenderTargetView);
		if (FAILED(result))
		{
			return false;
		}

		// Release pointer to the back buffer as we no longer need it.
		backBufferPtr->Release();
		backBufferPtr = 0;
	}

	{
		// Initialize the description of the depth buffer.
		D3D11_TEXTURE2D_DESC depthBufferDesc;
		ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

		// Set up the description of the depth buffer.
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
		// Create the texture for the depth buffer using the filled out description.
		result = m_device->CreateTexture2D(&depthBufferDesc, NULL, &view.DepthStencilBuffer);
		if (FAILED(result))
		{
			return false;
		}
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
		result = m_device->CreateDepthStencilState(&depthStencilDesc, &view.DepthStencilState);
		if (FAILED(result))
		{
			return false;
		}
	}

	{
		// Set the depth stencil state.
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		m_deviceContext->OMSetDepthStencilState(view.DepthStencilState, 1);

		// Initialize the depth stencil view.
		ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

		// Set up the depth stencil view description.
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		// Create the depth stencil view.
		result = m_device->CreateDepthStencilView(view.DepthStencilBuffer, &depthStencilViewDesc, &view.DepthStencilView);
		if (FAILED(result))
		{
			return false;
		}
	}

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	m_deviceContext->OMSetRenderTargets(1, &view.RenderTargetView, view.DepthStencilView);

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
		result = m_device->CreateRasterizerState(&rasterDesc, &view.RasterState);
		if (FAILED(result))
		{
			return false;
		}
	}

	{
		// Now set the rasterizer state.
		m_deviceContext->RSSetState(view.RasterState);

		// Setup the viewport for rendering.
		view.Viewport.Width = (float)screenWidth * (3.0f / 4.0f);
		view.Viewport.Height = (float)screenHeight * (3.0f / 4.0f);
		view.Viewport.MinDepth = 0.0f;
		view.Viewport.MaxDepth = 1.0f;
		view.Viewport.TopLeftX = 0.0f;
		view.Viewport.TopLeftY = 0.0f;

		// Create the viewport.
		m_deviceContext->RSSetViewports(1, &view.Viewport);

		// Setup the projection matrix.
		fieldOfView = 3.141592654f / 4.0f;
		screenAspect = (float)screenWidth / (float)screenHeight;

		// Create the projection matrix for 3D rendering.
		view.ProjectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);

		// Initialize the world matrix to the identity matrix.
		view.WorldMatrix = XMMatrixIdentity();

		// Create an orthographic projection matrix for 2D rendering.
		view.OrthoMatrix = XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, screenNear, screenDepth);
	}

	{
		D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;
		// Clear the second depth stencil state before setting the parameters.
		ZeroMemory(&depthDisabledStencilDesc, sizeof(depthDisabledStencilDesc));

		// Now create a second depth stencil state which turns off the Z buffer for 2D rendering.  The only difference is 
		// that DepthEnable is set to false, all other parameters are the same as the other depth stencil state.
		depthDisabledStencilDesc.DepthEnable = false;
		depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
		depthDisabledStencilDesc.StencilEnable = true;
		depthDisabledStencilDesc.StencilReadMask = 0xFF;
		depthDisabledStencilDesc.StencilWriteMask = 0xFF;
		depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Create the state using the device.
		result = m_device->CreateDepthStencilState(&depthDisabledStencilDesc, &view.DepthDisabledStencilState);
		if (FAILED(result))
		{
			return false;
		}
	}

	{
		D3D11_BLEND_DESC blendStateDescription;

		// Clear the blend state description.
		ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));

		// Create an alpha enabled blend state description.
		blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
		blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDescription.RenderTarget[0].RenderTargetWriteMask = 0x0f;

		// Create the blend state using the description.
		result = m_device->CreateBlendState(&blendStateDescription, &view.AlphaEnableBlendingState);
		if (FAILED(result))
		{
			return false;
		}

		// Modify the description to create an alpha disabled blend state description.
		blendStateDescription.RenderTarget[0].BlendEnable = FALSE;

		// Create the blend state using the description.
		result = m_device->CreateBlendState(&blendStateDescription, &view.AlphaDisableBlendingState);
		if (FAILED(result))
		{
			return false;
		}
	}

	Views[EnumViewType::eScene] = view;
}

void D3DClass::Shutdown()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if(m_swapChain)
	{
		m_swapChain->SetFullscreenState(false, NULL);
	}

	if(m_deviceContext)
	{
		m_deviceContext->Release();
		m_deviceContext = 0;
	}

	if(m_device)
	{
		m_device->Release();
		m_device = 0;
	}

	if(m_swapChain)
	{
		m_swapChain->Release();
		m_swapChain = 0;
	}

	for (auto &view : Views)
	{
		auto info = view.second;

		if(info.AlphaEnableBlendingState)
		{
			info.AlphaEnableBlendingState->Release();
			info.AlphaEnableBlendingState = 0;
		}

		if(info.AlphaDisableBlendingState)
		{
			info.AlphaDisableBlendingState->Release();
			info.AlphaDisableBlendingState = 0;
		}

		if(info.DepthDisabledStencilState)
		{
			info.DepthDisabledStencilState->Release();
			info.DepthDisabledStencilState = 0;
		}

		if(info.RasterState)
		{
			info.RasterState->Release();
			info.RasterState = 0;
		}

		if(info.DepthStencilView)
		{
			info.DepthStencilView->Release();
			info.DepthStencilView = 0;
		}

		if(info.DepthStencilState)
		{
			info.DepthStencilState->Release();
			info.DepthStencilState = 0;
		}

		if(info.DepthStencilBuffer)
		{
			info.DepthStencilBuffer->Release();
			info.DepthStencilBuffer = 0;
		}

		if(info.RenderTargetView)
		{
			info.RenderTargetView->Release();
			info.RenderTargetView = 0;
		}
	}

	return;
}


void D3DClass::BeginScene(EnumViewType type, float red, float green, float blue, float alpha)
{
	float color[4];


	// Setup the color to clear the buffer to.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// Clear the back buffer.
	m_deviceContext->ClearRenderTargetView(Views[type].RenderTargetView, color);
    
	// Clear the depth buffer.
	m_deviceContext->ClearDepthStencilView(Views[type].DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;
}


void D3DClass::EndScene()
{
	// Present the back buffer to the screen since rendering is complete.
	if(m_vsync_enabled)
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


ID3D11Device* D3DClass::GetDevice()
{
	return m_device;
}


ID3D11DeviceContext* D3DClass::GetDeviceContext()
{
	return m_deviceContext;
}


void D3DClass::GetProjectionMatrix(EnumViewType type, XMMATRIX& projectionMatrix)
{
	projectionMatrix = Views[type].ProjectionMatrix;
	return;
}


void D3DClass::GetWorldMatrix(EnumViewType type, XMMATRIX& worldMatrix)
{
	worldMatrix = Views[type].WorldMatrix;
	return;
}


void D3DClass::GetOrthoMatrix(EnumViewType type, XMMATRIX& orthoMatrix)
{
	orthoMatrix = Views[type].OrthoMatrix;
	return;
}

//
//void D3DClass::GetVideoCardInfo(char* cardName, int& memory)
//{
//	strcpy_s(cardName, 128, m_videoCardDescription);
//	memory = m_videoCardMemory;
//	return;
//}


void D3DClass::SetBackBufferRenderTarget(EnumViewType type)
{
	// Bind the render target view and depth stencil buffer to the output render pipeline.
	m_deviceContext->OMSetRenderTargets(1, &Views[type].RenderTargetView, Views[type].DepthStencilView);

	return;
}


void D3DClass::ResetViewport(EnumViewType type)
{
	// Set the viewport.
	m_deviceContext->RSSetViewports(1, &Views[type].Viewport);

	return;
}


void D3DClass::TurnZBufferOn(EnumViewType type)
{
	m_deviceContext->OMSetDepthStencilState(Views[type].DepthStencilState, 1);
	return;
}


void D3DClass::TurnZBufferOff(EnumViewType type)
{
	m_deviceContext->OMSetDepthStencilState(Views[type].DepthDisabledStencilState, 1);
	return;
}


void D3DClass::EnableAlphaBlending(EnumViewType type)
{
	float blendFactor[4];


	// Setup the blend factor.
	blendFactor[0] = 0.0f;
	blendFactor[1] = 0.0f;
	blendFactor[2] = 0.0f;
	blendFactor[3] = 0.0f;

	// Turn on the alpha blending.
	m_deviceContext->OMSetBlendState(Views[type].AlphaEnableBlendingState, blendFactor, 0xffffffff);

	return;
}


void D3DClass::DisableAlphaBlending(EnumViewType type)
{
	float blendFactor[4];


	// Setup the blend factor.
	blendFactor[0] = 0.0f;
	blendFactor[1] = 0.0f;
	blendFactor[2] = 0.0f;
	blendFactor[3] = 0.0f;

	// Turn off the alpha blending.
	m_deviceContext->OMSetBlendState(Views[type].AlphaDisableBlendingState, blendFactor, 0xffffffff);

	return;
}