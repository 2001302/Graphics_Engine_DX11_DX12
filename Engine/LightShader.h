#ifndef _SHADER
#define _SHADER

#include "IShader.h"

using namespace DirectX;

namespace Engine
{
	class LightShader : public IShader
	{
	public:
        MatrixBufferType vertexConstantBufferData;
        LightBufferType pixelConstantBufferData;
	};
}
#endif