#ifndef _SHADER
#define _SHADER

#include "ShaderFactory.h"

using namespace DirectX;

namespace Engine
{
	using Microsoft::WRL::ComPtr;

	class LightShader
	{
	public:

		bool Initialize(ComPtr<ID3D11Device>, HWND);
		void Shutdown();

		ID3D11VertexShader* m_vertexShader;
		ID3D11PixelShader* m_pixelShader;
		ID3D11InputLayout* m_layout;
		ID3D11SamplerState* m_sampleState;
		ID3D11Buffer* m_matrixBuffer;
		ID3D11Buffer* m_lightBuffer;
	};
}
#endif