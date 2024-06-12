#ifndef _PHONGSHADER
#define _PHONGSHADER

#include "shader.h"

using namespace DirectX;

namespace Engine
{
	/// <summary>
	/// 공유되어 사용되는 Phong Shader
	/// </summary>
	class PhongShader : public IShader
	{
	public:

	};

	/// <summary>
	/// Phong Shader에서 사용하는 공유되지 않는 Constant Buffer 정보
	/// </summary>
	class PhongShaderSource 
	{
	public:		
		ComPtr<ID3D11Buffer> vertex_constant_buffer;
		ComPtr<ID3D11Buffer> pixel_constant_buffer;
		VertexConstantBuffer vertex_constant_buffer_data;
		PixelConstantBuffer pixel_constant_buffer_data;
	};
}
#endif