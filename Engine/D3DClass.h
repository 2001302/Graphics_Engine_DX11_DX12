#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_

#include <d3d11.h>
#include <directxmath.h>

#include "Header.h"

using namespace DirectX;

namespace Engine 
{
	enum EnumViewType
	{
		eScene = 0,
		eGame = 1,
		ePannel = 2,
	};

	struct ViewInfo
	{
		ID3D11Texture2D* DepthStencilBuffer;
		ID3D11DepthStencilState* DepthStencilState;
		ID3D11DepthStencilView* DepthStencilView;
		ID3D11RasterizerState* RasterState;

		XMMATRIX ProjectionMatrix;
		XMMATRIX WorldMatrix;
		XMMATRIX OrthoMatrix;

		ID3D11DepthStencilState* DepthDisabledStencilState;
		ID3D11BlendState* AlphaEnableBlendingState;
		ID3D11BlendState* AlphaDisableBlendingState;

		ID3D11RenderTargetView* RenderTargetView;
		D3D11_VIEWPORT Viewport;
	};

	class D3DClass
	{
	private:
		D3DClass() {}
		D3DClass(const D3DClass& ref) {}
		D3DClass& operator=(const D3DClass& ref) {}
		~D3DClass() {}

		bool m_vsync_enabled;

		IDXGISwapChain* m_swapChain;
		ID3D11Device* m_device;
		ID3D11DeviceContext* m_deviceContext;

	public:
		static D3DClass& GetInstance() {
			static D3DClass instance;
			return instance;
		}

		bool Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear);
		bool InitializeGraphicDevice(int screenWidth, int screenHeight, HWND hwnd, bool fullscreen);
		bool InitializeMainScene(int screenWidth, int screenHeight, float screenDepth, float screenNear);
		void Shutdown();

		void BeginScene(EnumViewType type, float, float, float, float);
		void EndScene();

		ID3D11Device* GetDevice();
		ID3D11DeviceContext* GetDeviceContext();

		void GetProjectionMatrix(EnumViewType type, XMMATRIX& projectionMatrix);
		void GetWorldMatrix(EnumViewType type, XMMATRIX& worldMatrix);
		void GetOrthoMatrix(EnumViewType type, XMMATRIX& orthoMatrix);

		void SetBackBufferRenderTarget(EnumViewType type);
		void ResetViewport(EnumViewType type);

		void TurnZBufferOn(EnumViewType type);
		void TurnZBufferOff(EnumViewType type);

		void EnableAlphaBlending(EnumViewType type);
		void DisableAlphaBlending(EnumViewType type);

		std::map<EnumViewType, ViewInfo> Views;

	};
}

#endif