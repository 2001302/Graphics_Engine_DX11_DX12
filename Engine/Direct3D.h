#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_

#include "CommonStruct.h"

using namespace DirectX;

namespace Engine 
{
	using Microsoft::WRL::ComPtr;

	enum EnumViewType
	{
		eScene = 0,
		eGame = 1,
		ePannel = 2,
	};

	struct ViewInfo
	{
		ComPtr<ID3D11Texture2D> DepthStencilBuffer;
		ComPtr<ID3D11DepthStencilState> DepthStencilState;
		ComPtr<ID3D11DepthStencilView> DepthStencilView;
		ComPtr<ID3D11RasterizerState> RasterState;

		XMMATRIX ProjectionMatrix;
		XMMATRIX WorldMatrix;
		XMMATRIX OrthoMatrix;

		ID3D11RenderTargetView* RenderTargetView;
		D3D11_VIEWPORT Viewport;
	};

	class Direct3D
	{
	private:
		Direct3D() : m_vsync_enabled(false), m_swapChain(0), m_device(0), m_deviceContext(0) {}
		Direct3D(const Direct3D& x) : m_vsync_enabled(x.m_vsync_enabled), m_swapChain(x.m_swapChain), m_device(x.m_device), m_deviceContext(x.m_deviceContext) {}
		Direct3D& operator=(const Direct3D& ref) {}
		~Direct3D() {}

		bool m_vsync_enabled;

		ComPtr<ID3D11Device> m_device;
		ComPtr<IDXGISwapChain> m_swapChain;
		ComPtr<ID3D11DeviceContext> m_deviceContext;

	public:
		std::map<EnumViewType, ViewInfo> Views;

		static Direct3D& GetInstance() {
			static Direct3D instance;
			return instance;
		}

		bool Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen);
		bool InitializeDirect3D(int screenWidth, int screenHeight, HWND hwnd, bool fullscreen);
		bool InitializeMainScene(int screenWidth, int screenHeight);
		void SetViewPort(EnumViewType type, float x, float y, float width, float height);
		void Shutdown();

		void BeginScene(EnumViewType type, float, float, float, float);
		void EndScene();

		ComPtr<ID3D11Device>  GetDevice();
		ComPtr<ID3D11DeviceContext>  GetDeviceContext();

		void GetProjectionMatrix(EnumViewType type, XMMATRIX& projectionMatrix);
		void GetWorldMatrix(EnumViewType type, XMMATRIX& worldMatrix);
		void GetOrthoMatrix(EnumViewType type, XMMATRIX& orthoMatrix);

		void CreateVertexBuffer(std::vector<Engine::VertexType> vertices, ID3D11Buffer* vertexBuffer);
		void CreateIndexBuffer(std::vector<int> indices, ID3D11Buffer* indexBuffer);

		template <typename T_CONSTANT>
		void CreateConstantBuffer(const T_CONSTANT& constantBufferData,
			ComPtr<ID3D11Buffer>& constantBuffer) {
			D3D11_BUFFER_DESC cbDesc;
			cbDesc.ByteWidth = sizeof(constantBufferData);
			cbDesc.Usage = D3D11_USAGE_DYNAMIC;
			cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			cbDesc.MiscFlags = 0;
			cbDesc.StructureByteStride = 0;

			// Fill in the subresource data.
			D3D11_SUBRESOURCE_DATA InitData;
			InitData.pSysMem = &constantBufferData;
			InitData.SysMemPitch = 0;
			InitData.SysMemSlicePitch = 0;

			m_device->CreateBuffer(&cbDesc, &InitData, constantBuffer.GetAddressOf());
		}

		template <typename T_DATA>
		void UpdateBuffer(const T_DATA& bufferData, ComPtr<ID3D11Buffer>& buffer) {
			D3D11_MAPPED_SUBRESOURCE ms;
			m_deviceContext->Map(buffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
			memcpy(ms.pData, &bufferData, sizeof(bufferData));
			m_deviceContext->Unmap(buffer.Get(), NULL);
		}
	};
}
#endif