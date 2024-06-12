#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_

#include "common_struct.h"
#include "env.h"

using namespace DirectX;

namespace Engine 
{
	using Microsoft::WRL::ComPtr;
	
	class Direct3D
	{
	private:
		Direct3D() : m_vsync_enabled(false), m_swapChain(0), m_device(0), m_deviceContext(0), viewport(D3D11_VIEWPORT()) {}
		Direct3D(const Direct3D& x) : m_vsync_enabled(x.m_vsync_enabled), m_swapChain(x.m_swapChain), m_device(x.m_device), m_deviceContext(x.m_deviceContext), viewport(x.viewport){}
		Direct3D& operator=(const Direct3D& ref) {}
		~Direct3D() {}

		bool m_vsync_enabled;
		UINT numQualityLevels = 0;

		ComPtr<ID3D11Device> m_device;
		ComPtr<IDXGISwapChain> m_swapChain;
		ComPtr<ID3D11DeviceContext> m_deviceContext;

	public:
		static Direct3D& GetInstance() {
			static Direct3D instance;
			return instance;
		}
		bool Init(Env* env, bool vsync, HWND hwnd, bool fullscreen);
		bool InitDirect3D(int screenWidth, int screenHeight, HWND hwnd, bool fullscreen);
		bool InitMainScene(int screenWidth, int screenHeight);

		void SetViewPort(float x, float y, float width, float height);
		void Shutdown();

		void BeginScene(float, float, float, float);
		void EndScene();

		ComPtr<ID3D11Device>  GetDevice();
		ComPtr<ID3D11DeviceContext>  GetDeviceContext();

		ComPtr<ID3D11Texture2D> depthStencilBuffer;
		ComPtr<ID3D11DepthStencilState> depthStencilState;
		ComPtr<ID3D11DepthStencilView> depthStencilView;

		ComPtr<ID3D11RasterizerState> solidRasterizerSate;
		ComPtr<ID3D11RasterizerState> wireRasterizerSate;

		ID3D11RenderTargetView* renderTargetView;
		D3D11_VIEWPORT viewport;
	};
}
#endif