#ifndef _SHADERFACTORY
#define _SHADERFACTORY

#include "IShader.h"

namespace Engine
{
	class ShaderFactory
	{
	public:
		void Initialize() {};
		void Select() {};
		void Render() {};

		std::map<int, std::shared_ptr<IShader>> shaders;
	};
}
#endif